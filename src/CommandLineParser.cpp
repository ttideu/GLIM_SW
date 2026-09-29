#include "CommandLineParser.hpp"

#include <stdexcept>
#include <string>

namespace
{
    // 옵션 뒤에 필요한 값이 있는지 확인하고 반환합니다.
    std::string requireValue(int argc, char* argv[], int& index, const std::string& option)
    {
        if (index + 1 >= argc)
        {
            throw std::invalid_argument("missing value for " + option);
        }
        ++index;
        return argv[index];
    }

    // 문자열 옵션 값을 정수로 변환합니다.
    int parseInteger(const std::string& text, const std::string& option)
    {
        std::size_t used = 0;
        int value = 0;

        try
        {
            value = std::stoi(text, &used);
        }
        catch (const std::exception&)
        {
            throw std::invalid_argument("invalid integer for " + option + ": " + text);
        }

        if (used != text.size())
        {
            throw std::invalid_argument("invalid integer for " + option + ": " + text);
        }

        return value;
    }
}

// 인자를 읽어 CommandLineOptions로 정리합니다.
CommandLineOptions CommandLineParser::parse(int argc, char* argv[])
{
    CommandLineOptions options;

    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];

        if (arg == "--input")
        {
            options.inputPath = requireValue(argc, argv, i, arg);
        }
        else if (arg == "--output")
        {
            options.outputPath = requireValue(argc, argv, i, arg);
        }
        else if (arg == "--filter")
        {
            options.filter = requireValue(argc, argv, i, arg);
        }
        else if (arg == "--pipeline")
        {
            options.pipeline = requireValue(argc, argv, i, arg);
        }
        else if (arg == "--threshold")
        {
            options.threshold = parseInteger(requireValue(argc, argv, i, arg), arg);
        }
        else if (arg == "--help" || arg == "-h")
        {
            throw std::invalid_argument(usage());
        }
        else
        {
            throw std::invalid_argument("unknown option: " + arg);
        }
    }

    if (options.inputPath.empty())
    {
        throw std::invalid_argument("--input is required");
    }
    if (options.outputPath.empty())
    {
        throw std::invalid_argument("--output is required");
    }
    if (options.filter.has_value() == options.pipeline.has_value())
    {
        throw std::invalid_argument("specify exactly one of --filter or --pipeline");
    }

    return options;
}

// 실행 예시와 사용 가능한 옵션을 반환합니다.
std::string CommandLineParser::usage()
{
    return
        "Usage:\n"
        "  ImageProcessor.exe --input input.bmp --output output.bmp --filter grayscale\n"
        "  ImageProcessor.exe --input input.bmp --output output.bmp --filter threshold --threshold 128\n"
        "  ImageProcessor.exe --input input.bmp --output output.bmp --filter blur\n"
        "  ImageProcessor.exe --input input.bmp --output output.bmp --filter sharpen\n"
        "  ImageProcessor.exe --input input.bmp --output output.bmp --filter flip-h\n"
        "  ImageProcessor.exe --input input.bmp --output output.bmp --filter flip-v\n"
        "  ImageProcessor.exe --input input.bmp --output output.bmp --pipeline \"grayscale,blur,threshold:128\"\n";
}
