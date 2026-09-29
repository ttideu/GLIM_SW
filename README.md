# CLI Image Processor

## 구현 범위

* Grayscale 변환
* Thresholding
* 3x3 Convolution

  * Blur
  * Sharpen
* 좌우 반전 / 상하 반전

추가로 FilterBase 추상 클래스를 기준으로 각 필터를 분리했고, 여러 필터를 순서대로 실행할 수 있는 Pipeline을 구현했습니다.





## 개발 환경

* Windows
* Visual Studio 2022
* C++17 이상





## 빌드

Visual Studio 2022에서 CMake 프로젝트로 빌드할 수 있습니다.



cmake -S . -B build
cmake --build build --config Release




## 사용법

### Grayscale
ImageProcessor.exe --input input.bmp --output result.bmp --filter grayscale


### Threshold
ImageProcessor.exe --input input.bmp --output result.bmp --filter threshold --threshold 128



Threshold 값은 0\~255 범위만 허용합니다.



### Blur
ImageProcessor.exe --input input.bmp --output result.bmp --filter blur


### Sharpen
ImageProcessor.exe --input input.bmp --output result.bmp --filter sharpen


### 좌우 / 상하 반전
ImageProcessor.exe --input input.bmp --output result.bmp --filter flip-h
ImageProcessor.exe --input input.bmp --output result.bmp --filter flip-v


### Pipeline
ImageProcessor.exe --input input.bmp --output result.bmp --pipeline "grayscale,blur,threshold:128"


위 Pipeline은 다음 순서로 실행됩니다. Grayscale -> Blur -> Threshold(128)





## 구현 설명

### ImageBuffer

영상은 내부에서 BGR 3채널, top-down, row-major 형태로 저장합니다. 한 픽셀은 3 byte이며 'rowPtr(y)'를 통해 각 행의 시작 주소에 접근합니다.



### Grayscale

각 픽셀의 BGR 값으로 다음 가중합을 계산하고 동일 값을 B/G/R 채널에 기록합니다.
Gray = 0.299R + 0.587G + 0.114B



### Threshold

픽셀의 밝기를 Grayscale과 같은 방식으로 계산한 뒤 임계값보다 작으면 0, 그렇지 않으면 255로 변환합니다. 

단일 필터로도 동작하도록 입력 이미지가 미리 Grayscale이라는 가정은 두지 않았습니다.



### 3x3 Convolution

Blur와 Sharpen은 동일한 Convolution 연산을 사용하고 Kernel만 다르게 구성했습니다.

### Blur kernel:
```bash
1 1 1
1 1 1   / 9
1 1 1
```

### Sharpen kernel:
```bash
 0 -1  0
-1  5 -1
 0 -1  0
```

Convolution은 주변 픽셀을 참조하므로 처리 중 변경된 값이 다음 계산에 영향을 주지 않도록 원본 'ImageBuffer'를 복사하여 읽기 전용 source로 사용합니다. 

3x3 Kernel이 이미지 밖을 참조하는 문제를 피하기 위해 가장자리 1 pixel은 원본 값을 유지합니다.



### Flip

좌우 반전은 oppositeX = width - 1 - x를 이용하여 한 행의 좌우 픽셀을 절반까지만 교환합니다. 

상하 반전은 위아래 행을 std::swap\_ranges로 교환합니다.



### Filter 구조와 Pipeline

모든 필터는 다음 공통 인터페이스를 구현합니다.


class FilterBase
{
public:
    virtual \~FilterBase() = default;
    virtual void apply(ImageBuffer\& image) const = 0;
};


Pipeline은 std::unique\_ptr<FilterBase>를 순서대로 보관한 뒤 동일한 apply() 인터페이스로 실행합니다. 

필터별 알고리즘은 분리하면서 Pipeline 실행 코드는 구체적인 필터 타입에 의존하지 않도록 구성했습니다.





## 예외 처리

아래 입력은 오류로 처리합니다.

* 입력/출력 경로 누락
* '--filter', '--pipeline'을 동시에 사용하거나 둘 다 사용하지 않은 경우
* 지원하지 않는 필터명
* Threshold 범위가 0\~255를 벗어난 경우
* 24-bit 비압축 BMP가 아닌 경우

