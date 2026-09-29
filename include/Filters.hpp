#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ImageBuffer.hpp"

// 3x3 convolution에 사용할 커널 자료형입니다.
using Kernel3x3 = std::array<std::array<int, 3>, 3>;

// 모든 필터를 같은 방식으로 apply()할 수 있도록 공통 인터페이스를 정의합니다.
class FilterBase
{
public:
    virtual ~FilterBase() = default;
    virtual void apply(ImageBuffer& image) const = 0;
};

// 컬러 이미지를 grayscale로 변환하는 필터입니다.
class GrayscaleFilter final : public FilterBase
{
public:
    void apply(ImageBuffer& image) const override;
};

// 지정한 임계값을 기준으로 흑백 영상을 만드는 필터입니다.
class ThresholdFilter final : public FilterBase
{
public:
    explicit ThresholdFilter(std::uint8_t thresholdValue);
    void apply(ImageBuffer& image) const override;

private:
    std::uint8_t m_thresholdValue;
};

// 이미지를 좌우로 반전하는 필터입니다.
class FlipHorizontalFilter final : public FilterBase
{
public:
    void apply(ImageBuffer& image) const override;
};

// 이미지를 상하로 반전하는 필터입니다.
class FlipVerticalFilter final : public FilterBase
{
public:
    void apply(ImageBuffer& image) const override;
};

// 전달받은 3x3 커널로 convolution 연산을 수행하는 필터입니다.
class ConvolutionFilter final : public FilterBase
{
public:
    ConvolutionFilter(Kernel3x3 kernel, int divisor);
    void apply(ImageBuffer& image) const override;

private:
    Kernel3x3 m_kernel;
    int m_divisor;
};

// 여러 필터를 등록된 순서대로 실행하는 클래스입니다.
class FilterPipeline
{
public:
    void add(std::unique_ptr<FilterBase> filter);
    void apply(ImageBuffer& image) const;
    bool empty() const noexcept;

private:
    std::vector<std::unique_ptr<FilterBase>> m_filters;
};

// 문자열로 받은 필터 이름을 실제 필터 객체로 생성합니다.
std::unique_ptr<FilterBase> createFilter(
    const std::string& filterText,
    const std::optional<int>& commandLineThreshold = std::nullopt);

// 콤마로 구분된 문자열을 FilterPipeline으로 구성합니다.
FilterPipeline buildPipeline(const std::string& pipelineText);
