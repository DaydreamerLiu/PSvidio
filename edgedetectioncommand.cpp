#include "edgedetectioncommand.h"
#include <cmath>

EdgeDetectionCommand::EdgeDetectionCommand(const QImage &originalImage, int threshold)
    : ImageCommand(originalImage, "边缘检测")
    , m_threshold(threshold)
{
}

QImage EdgeDetectionCommand::execute()
{
    return sobelEdgeDetection(m_originalImage, m_threshold);
}

int EdgeDetectionCommand::threshold() const
{
    return m_threshold;
}

QImage EdgeDetectionCommand::toGrayscale(const QImage &image)
{
    QImage grayImage(image.width(), image.height(), QImage::Format_Grayscale8);

    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor pixelColor = image.pixelColor(x, y);
            int grayValue = (pixelColor.red() + pixelColor.green() + pixelColor.blue()) / 3;
            grayImage.setPixel(x, y, grayValue);
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
        for (int x = 1; x < width - 1; ++x) {
            int gradientX = 0;
            int gradientY = 0;

            // 计算x和y方向的梯度
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int nx = x + kx;
                    int ny = y + ky;
                    int pixelValue = grayImage.pixel(nx, ny) & 0xFF;
                    
                    gradientX += pixelValue * sobelX[ky + 1][kx + 1];
                    gradientY += pixelValue * sobelY[ky + 1][kx + 1];
                }
            }

            // 计算梯度幅值
            int gradientMagnitude = qRound(sqrt(gradientX * gradientX + gradientY * gradientY));
            
            // 根据阈值进行二值化处理
            int edgeValue = (gradientMagnitude > threshold) ? 255 : 0;

            // 设置边缘检测结果
            resultImage.setPixel(x, y, edgeValue);
        }
    }

    return resultImage;
}
