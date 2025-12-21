#include "edgedetectioncommand.h"
#include <cmath>
#include <algorithm>

EdgeDetectionCommand::EdgeDetectionCommand(const QImage &originalImage, int threshold)
    : ImageCommand(originalImage, "边缘检测")
    , m_threshold(threshold)
{
}

QImage EdgeDetectionCommand::execute()
{
    return sobelEdgeDetection(m_inputImage, m_threshold);
}

int EdgeDetectionCommand::threshold() const
{
    return m_threshold;
}

QImage EdgeDetectionCommand::toGrayscale(const QImage &image)
{
    QImage grayImage(image.width(), image.height(), QImage::Format_Grayscale8);

    // 转换为RGB32格式以便快速访问像素
    QImage rgbImage = image.convertToFormat(QImage::Format_RGB32);

    for (int y = 0; y < image.height(); ++y) {
        QRgb *rgbLine = reinterpret_cast<QRgb*>(rgbImage.scanLine(y));
        uchar *grayLine = grayImage.scanLine(y);
        
        for (int x = 0; x < image.width(); ++x) {
            QRgb pixel = rgbLine[x];
            // 使用更高效的灰度转换公式，考虑人眼对不同颜色的敏感度
            int grayValue = qGray(pixel);
            grayLine[x] = static_cast<uchar>(grayValue);
        }
    }

    return grayImage;
}

QImage EdgeDetectionCommand::sobelEdgeDetection(const QImage &image, int threshold)
{
    // 转换为灰度图
    QImage grayImage = toGrayscale(image);
    QImage resultImage(grayImage.width(), grayImage.height(), QImage::Format_Grayscale8);

    // Sobel算子
    int sobelX[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    int sobelY[3][3] = {
        {-1, -2, -1},
        {0, 0, 0},
        {1, 2, 1}
    };

    int width = grayImage.width();
    int height = grayImage.height();

    // 应用Sobel算子
    for (int y = 1; y < height - 1; ++y) {
        // 获取当前行和上下行的指针
        const uchar *prevLine = grayImage.scanLine(y - 1);
        const uchar *currLine = grayImage.scanLine(y);
        const uchar *nextLine = grayImage.scanLine(y + 1);
        uchar *resultLine = resultImage.scanLine(y);
        
        for (int x = 1; x < width - 1; ++x) {
            int gradientX = 0;
            int gradientY = 0;

            // 计算x和y方向的梯度
            gradientX = -prevLine[x-1] + prevLine[x+1] - 2*currLine[x-1] + 2*currLine[x+1] - nextLine[x-1] + nextLine[x+1];
            gradientY = -prevLine[x-1] - 2*prevLine[x] - prevLine[x+1] + nextLine[x-1] + 2*nextLine[x] + nextLine[x+1];

            // 计算梯度幅值（使用近似计算避免sqrt，提高性能）
            // int gradientMagnitude = qRound(sqrt(gradientX * gradientX + gradientY * gradientY));
            
            // 使用更快速的近似计算：|Gx| + |Gy| 或 max(|Gx|, |Gy|)
            int gradientMagnitude = std::abs(gradientX) + std::abs(gradientY);
            
            // 根据阈值进行二值化处理
            int edgeValue = (gradientMagnitude > threshold) ? 255 : 0;

            // 设置边缘检测结果
            resultLine[x] = static_cast<uchar>(edgeValue);
        }
    }

    return resultImage;
}
