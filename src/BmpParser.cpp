#include "BmpParser.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace
{
#pragma pack(push, 1)
    struct BmpFileHeader
    {
        std::uint16_t signature;
        std::uint32_t fileSize;
        std::uint16_t reserved1;
        std::uint16_t reserved2;
        std::uint32_t pixelOffset;
    };

    struct BmpInfoHeader
    {
        std::uint32_t headerSize;
        std::int32_t width;
        std::int32_t height;
        std::uint16_t planes;
        std::uint16_t bitsPerPixel;
        std::uint32_t compression;
        std::uint32_t imageSize;
        std::int32_t xPixelsPerMeter;
        std::int32_t yPixelsPerMeter;
        std::uint32_t colorsUsed;
        std::uint32_t importantColors;
    };
#pragma pack(pop)

    constexpr std::uint16_t kBmpSignature = 0x4D42;
    constexpr std::uint32_t kBiRgb = 0;

    std::size_t paddedRowBytes(int width)
    {
        const std::size_t raw = static_cast<std::size_t>(width) * 3U;
        return (raw + 3U) & ~std::size_t{3U};
    }

    void ensureRead(std::istream& stream, char* data, std::streamsize size,
                    const char* message)
    {
        stream.read(data, size);
        if (!stream)
        {
            throw std::runtime_error(message);
        }
    }
}

ImageBuffer BmpParser::load(const std::string& path)
{
    std::ifstream input(path, std::ios::binary);
    if (!input)
    {
        throw std::runtime_error("failed to open input BMP: " + path);
    }

    BmpFileHeader fileHeader{};
    BmpInfoHeader infoHeader{};

    ensureRead(input,
               reinterpret_cast<char*>(&fileHeader),
               sizeof(fileHeader),
               "failed to read BMP file header");

    ensureRead(input,
               reinterpret_cast<char*>(&infoHeader),
               sizeof(infoHeader),
               "failed to read BMP info header");

    if (fileHeader.signature != kBmpSignature)
    {
        throw std::runtime_error("input file is not a BMP file");
    }

    if (infoHeader.headerSize < sizeof(BmpInfoHeader) ||
        infoHeader.width <= 0 ||
        infoHeader.height == 0 ||
        infoHeader.planes != 1 ||
        infoHeader.bitsPerPixel != 24 ||
        infoHeader.compression != kBiRgb)
    {
        throw std::runtime_error(
            "only uncompressed 24-bit BMP files are supported");
    }

    const int width = infoHeader.width;
    const int height = infoHeader.height > 0
        ? infoHeader.height
        : -infoHeader.height;
    const bool bottomUp = infoHeader.height > 0;

    ImageBuffer image(width, height);
    const std::size_t srcRowBytes = paddedRowBytes(width);
    const std::size_t dstRowBytes = image.rowBytes();
    std::vector<std::uint8_t> row(srcRowBytes);

    input.seekg(static_cast<std::streamoff>(fileHeader.pixelOffset), std::ios::beg);
    if (!input)
    {
        throw std::runtime_error("invalid BMP pixel offset");
    }

    for (int fileY = 0; fileY < height; ++fileY)
    {
        ensureRead(input,
                   reinterpret_cast<char*>(row.data()),
                   static_cast<std::streamsize>(srcRowBytes),
                   "unexpected end of BMP pixel data");

        const int imageY = bottomUp ? (height - 1 - fileY) : fileY;
        std::uint8_t* dst = image.rowPtr(imageY);
        std::copy(row.begin(), row.begin() + static_cast<std::ptrdiff_t>(dstRowBytes), dst);
    }

    return image;
}

void BmpParser::save(const std::string& path, const ImageBuffer& image)
{
    if (image.width() <= 0 || image.height() <= 0)
    {
        throw std::runtime_error("cannot save an empty image");
    }

    const std::size_t rawRowBytes = image.rowBytes();
    const std::size_t fileRowBytes = paddedRowBytes(image.width());
    const std::size_t pixelBytes = fileRowBytes *
        static_cast<std::size_t>(image.height());

    BmpFileHeader fileHeader{};
    fileHeader.signature = kBmpSignature;
    fileHeader.pixelOffset = sizeof(BmpFileHeader) + sizeof(BmpInfoHeader);
    fileHeader.fileSize = static_cast<std::uint32_t>(
        fileHeader.pixelOffset + pixelBytes);

    BmpInfoHeader infoHeader{};
    infoHeader.headerSize = sizeof(BmpInfoHeader);
    infoHeader.width = image.width();
    infoHeader.height = image.height();
    infoHeader.planes = 1;
    infoHeader.bitsPerPixel = 24;
    infoHeader.compression = kBiRgb;
    infoHeader.imageSize = static_cast<std::uint32_t>(pixelBytes);

    std::ofstream output(path, std::ios::binary);
    if (!output)
    {
        throw std::runtime_error("failed to open output BMP: " + path);
    }

    output.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
    output.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));

    const std::array<std::uint8_t, 3> padding{0, 0, 0};
    const std::size_t paddingBytes = fileRowBytes - rawRowBytes;

    for (int y = image.height() - 1; y >= 0; --y)
    {
        const std::uint8_t* row = image.rowPtr(y);
        output.write(reinterpret_cast<const char*>(row),
                     static_cast<std::streamsize>(rawRowBytes));

        if (paddingBytes > 0)
        {
            output.write(reinterpret_cast<const char*>(padding.data()),
                         static_cast<std::streamsize>(paddingBytes));
        }
    }

    if (!output)
    {
        throw std::runtime_error("failed while writing output BMP");
    }
}
