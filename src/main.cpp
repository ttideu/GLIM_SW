#include <exception>
#include <iostream>

#include "BmpParser.hpp"
#include "CommandLineParser.hpp"
#include "Filters.hpp"

int main(int argc, char* argv[])
{
    try
    {
        // 명령행 옵션을 읽습니다.
        const CommandLineOptions options = CommandLineParser::parse(argc, argv);

        // 입력 BMP를 메모리로 불러옵니다.
        ImageBuffer image = BmpParser::load(options.inputPath);

        // pipeline이 있으면 여러 필터를 순서대로 적용하고, 아니면 단일 필터를 적용합니다.
        if (options.pipeline.has_value())
        {
            FilterPipeline pipeline = buildPipeline(*options.pipeline);
            pipeline.apply(image);
        }
        else
        {
            auto filter = createFilter(*options.filter, options.threshold);
            filter->apply(image);
        }

        // 처리된 이미지를 BMP로 저장합니다.
        BmpParser::save(options.outputPath, image);

        std::cout << "Saved: " << options.outputPath << '\n';
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n\n"
                  << CommandLineParser::usage();
        return 1;
    }
}
