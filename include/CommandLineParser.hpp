#pragma once

#include <optional>
#include <string>

// 실행 시 입력받은 명령행 옵션을 저장합니다.
struct CommandLineOptions
{
    std::string inputPath;
    std::string outputPath;
    std::optional<std::string> filter;
    std::optional<std::string> pipeline;
    std::optional<int> threshold;
};

// 명령행 인자를 읽고 프로그램에서 사용할 옵션으로 변환합니다.
class CommandLineParser
{
public:
    // argc, argv를 분석해 옵션 값을 반환합니다.
    static CommandLineOptions parse(int argc, char* argv[]);
    // 사용 가능한 실행 형식을 문자열로 반환합니다.
    static std::string usage();
};
