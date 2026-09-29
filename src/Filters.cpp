#include "Filters.hpp"

#include <algorithm>
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace
{
    // BGR 값을 밝기 기준의 grayscale 값으로 변환합니다.
    std::uint8_t toGray(std::uint8_t b, std::uint8_t g, std::uint8_t r)
    {
        const double gray =
            0.114 * static_cast<double>(b) +
            0.587 * static_cast<double>(g) +
            0.299 * static_cast<double>(r);

        return static_cast<std::uint8_t>(gray);
    }

    // threshold 값이 0~255 범위인지 확인합니다.
    std::uint8_t checkedThreshold(int value)
    {
        if (value < 0 || value > 255)
        {
            throw std::invalid_argument("threshold must be between 0 and 255");
        }
        return static_cast<std::uint8_t>(value);
    }

    // 문자열 앞뒤의 공백을 제거합니다.
    std::string trim(const std::string& text)
    {
        const auto begin = text.find_first_not_of(" \t\r\n");
        if (begin == std::string::npos)
        {
            return {};
        }
        const auto end = text.find_last_not_of(" \t\r\n");
        return text.substr(begin, end - begin + 1);
    }

    // 문자열 전체가 정수인지 확인하고 변환합니다.
    int parseIntStrict(const std::string& text, const std::string& name)
    {
        std::size_t used = 0;
        int value = 0;

        try
        {
            value = std::stoi(text, &used);
        }
        catch (const std::exception&)
        {
            throw std::invalid_argument("invalid " + name + ": " + text);
        }

        if (used != text.size())
        {
            throw std::invalid_argument("invalid " + name + ": " + text);
        }

        return value;
    }
}

// grayscale 필터를 적용합니다.
void GrayscaleFilter::apply(ImageBuffer& image) const
{
    for (int y = 0; y < image.height(); ++y)
    {
        std::uint8_t* row = image.rowPtr(y);

        for (int x = 0; x < image.width(); ++x)
        {
            const std::uint8_t b = row[x * 3 + 0];
            const std::uint8_t g = row[x * 3 + 1];
            const std::uint8_t r = row[x * 3 + 2];
            const std::uint8_t gray = toGray(b, g, r);

            row[x * 3 + 0] = gray;
            row[x * 3 + 1] = gray;
            row[x * 3 + 2] = gray;
        }
    }
}

// 사용할 threshold 값을 저장합니다.
ThresholdFilter::ThresholdFilter(std::uint8_t thresholdValue)
    : m_thresholdValue(thresholdValue)
{
}

// threshold 필터를 적용합니다.
void ThresholdFilter::apply(ImageBuffer& image) const
{
    for (int y = 0; y < image.height(); ++y)
    {
        std::uint8_t* row = image.rowPtr(y);

        for (int x = 0; x < image.width(); ++x)
        {
            const std::uint8_t b = row[x * 3 + 0];
            const std::uint8_t g = row[x * 3 + 1];
            const std::uint8_t r = row[x * 3 + 2];
            const std::uint8_t gray = toGray(b, g, r);
            const std::uint8_t value = gray < m_thresholdValue ? 0 : 255;

            row[x * 3 + 0] = value;
            row[x * 3 + 1] = value;
            row[x * 3 + 2] = value;
        }
    }
}

// 이미지를 좌우로 반전합니다.
void FlipHorizontalFilter::apply(ImageBuffer& image) const
{
    for (int y = 0; y < image.height(); ++y)
    {
        std::uint8_t* row = image.rowPtr(y);

        for (int x = 0; x < image.width() / 2; ++x)
        {
            const int oppositeX = image.width() - 1 - x;

            for (int channel = 0; channel < 3; ++channel)
            {
                std::swap(
                    row[x * 3 + channel],
                    row[oppositeX * 3 + channel]);
            }
        }
    }
}

// 이미지를 상하로 반전합니다.
void FlipVerticalFilter::apply(ImageBuffer& image) const
{
    const std::size_t rowBytes = image.rowBytes();

    for (int y = 0; y < image.height() / 2; ++y)
    {
        const int oppositeY = image.height() - 1 - y;
        std::uint8_t* topRow = image.rowPtr(y);
        std::uint8_t* bottomRow = image.rowPtr(oppositeY);

        std::swap_ranges(topRow, topRow + rowBytes, bottomRow);
    }
}

// convolution에 사용할 커널과 나눗값을 저장합니다.
ConvolutionFilter::ConvolutionFilter(Kernel3x3 kernel, int divisor)
    : m_kernel(std::move(kernel)), m_divisor(divisor)
{
    if (m_divisor == 0)
    {
        throw std::invalid_argument("convolution divisor must not be zero");
    }
}

// 3x3 convolution 필터를 적용합니다.
void ConvolutionFilter::apply(ImageBuffer& image) const
{
    if (image.width() < 3 || image.height() < 3)
    {
        return;
    }

    // 처리된 값이 다음 계산에 섞이지 않도록 원본 이미지를 복사해서 사용합니다.
    const ImageBuffer source = image;

    for (int y = 1; y < image.height() - 1; ++y)
    {
        std::uint8_t* dstRow = image.rowPtr(y);

        for (int x = 1; x < image.width() - 1; ++x)
        {
            for (int channel = 0; channel < 3; ++channel)
            {
                int sum = 0;

                for (int ky = -1; ky <= 1; ++ky)
                {
                    const std::uint8_t* srcRow = source.rowPtr(y + ky);

                    for (int kx = -1; kx <= 1; ++kx)
                    {
                        const int pixel = srcRow[(x + kx) * 3 + channel];
                        const int weight = m_kernel[ky + 1][kx + 1];
                        sum += pixel * weight;
                    }
                }

                int result = sum / m_divisor;
                result = std::clamp(result, 0, 255);
                dstRow[x * 3 + channel] = static_cast<std::uint8_t>(result);
            }
        }
    }
}

// pipeline에 필터를 추가합니다.
void FilterPipeline::add(std::unique_ptr<FilterBase> filter)
{
    if (!filter)
    {
        throw std::invalid_argument("cannot add a null filter");
    }
    m_filters.push_back(std::move(filter));
}

// 등록된 필터를 순서대로 적용합니다.
void FilterPipeline::apply(ImageBuffer& image) const
{
    for (const auto& filter : m_filters)
    {
        filter->apply(image);
    }
}

bool FilterPipeline::empty() const noexcept
{
    return m_filters.empty();
}

// 입력 문자열에 맞는 필터 객체를 생성합니다.
std::unique_ptr<FilterBase> createFilter(const std::string& filterText, 
const std::optional<int>& commandLineThreshold)
{
    const std::string token = trim(filterText);

    if (token == "grayscale")
    {
        return std::make_unique<GrayscaleFilter>();
    }

    if (token == "blur")
    {
        const Kernel3x3 kernel{{
            {{1, 1, 1}},
            {{1, 1, 1}},
            {{1, 1, 1}}
        }};
        return std::make_unique<ConvolutionFilter>(kernel, 9);
    }

    if (token == "sharpen")
    {
        const Kernel3x3 kernel{{
            {{ 0, -1,  0}},
            {{-1,  5, -1}},
            {{ 0, -1,  0}}
        }};
        return std::make_unique<ConvolutionFilter>(kernel, 1);
    }

    if (token == "flip-h" || token == "flip-horizontal")
    {
        return std::make_unique<FlipHorizontalFilter>();
    }

    if (token == "flip-v" || token == "flip-vertical")
    {
        return std::make_unique<FlipVerticalFilter>();
    }

    if (token == "threshold")
    {
        if (!commandLineThreshold.has_value())
        {
            throw std::invalid_argument(
                "threshold filter requires --threshold 0..255");
        }
        return std::make_unique<ThresholdFilter>(checkedThreshold(*commandLineThreshold));
    }

    const std::string prefix = "threshold:";
    if (token.rfind(prefix, 0) == 0)
    {
        const std::string valueText = trim(token.substr(prefix.size()));
        if (valueText.empty())
        {
            throw std::invalid_argument("threshold value is missing");
        }

        return std::make_unique<ThresholdFilter>(
            checkedThreshold(parseIntStrict(valueText, "threshold")));
    }

    throw std::invalid_argument("unknown filter: " + token);
}

// pipeline 문자열을 필터 목록으로 변환합니다.
FilterPipeline buildPipeline(const std::string& pipelineText)
{
    FilterPipeline pipeline;
    std::stringstream stream(pipelineText);
    std::string token;

    while (std::getline(stream, token, ','))
    {
        token = trim(token);
        if (token.empty())
        {
            throw std::invalid_argument("pipeline contains an empty filter");
        }
        pipeline.add(createFilter(token));
    }

    if (pipeline.empty())
    {
        throw std::invalid_argument("pipeline must contain at least one filter");
    }

    return pipeline;
}
