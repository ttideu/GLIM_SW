#pragma once

#include <string>
#include "ImageBuffer.hpp"

class BmpParser
{
public:
    static ImageBuffer load(const std::string& path);
    static void save(const std::string& path, const ImageBuffer& image);
};
