#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

// 24비트 BGR 픽셀 데이터를 메모리에 보관하는 클래스입니다.
class ImageBuffer
{
public:
    ImageBuffer() = default;

    // 이미지 크기에 맞게 픽셀 버퍼를 생성합니다.
    ImageBuffer(int width, int height)
        : m_width(width),
          m_height(height),
          m_data(checkedDataSize(width, height))
    {
    }

    int width() const noexcept { return m_width; }
    int height() const noexcept { return m_height; }

    std::uint8_t* rowPtr(int y)
    {
        checkRow(y);
        return m_data.data() + static_cast<std::size_t>(y) * rowBytes();
    }

    const std::uint8_t* rowPtr(int y) const
    {
        checkRow(y);
        return m_data.data() + static_cast<std::size_t>(y) * rowBytes();
    }

    std::size_t rowBytes() const noexcept
    {
        return static_cast<std::size_t>(m_width) * 3U;
    }

private:
    static std::size_t checkedDataSize(int width, int height)
    {
        if (width <= 0 || height <= 0)
        {
            throw std::invalid_argument("image width and height must be positive");
        }

        return static_cast<std::size_t>(width) *
               static_cast<std::size_t>(height) * 3U;
    }

    void checkRow(int y) const
    {
        if (y < 0 || y >= m_height)
        {
            throw std::out_of_range("row index out of range");
        }
    }

    int m_width = 0;
    int m_height = 0;
    std::vector<std::uint8_t> m_data;
};
