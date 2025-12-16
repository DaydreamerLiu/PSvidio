#include "meanfiltercommand.h"

MeanFilterCommand::MeanFilterCommand(const QImage &originalImage)
    : ImageCommand(originalImage, "3×3均值滤波")
{
}

QImage MeanFilterCommand::execute()
{
    QImage resultImage = m_originalImage.copy();
    int width = resultImage.width();
    int height = resultImage.height();

    // 3×3均值滤波
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int sumR = 0, sumG = 0, sumB = 0;
            int count = 0;

            // 遍历3×3邻域
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int nx = x + kx;
                    int ny = y + ky;

                    // 检查边界
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        QColor pixelColor = m_originalImage.pixelColor(nx, ny);
                        sumR += pixelColor.red();
                        sumG += pixelColor.green();
                        sumB += pixelColor.blue();
                        count++;
                    }
                }
            }

            // 计算平均值
            int avgR = sumR / count;
            int avgG = sumG / count;
            int avgB = sumB / count;

            // 设置滤波后的像素
            resultImage.setPixelColor(x, y, QColor(avgR, avgG, avgB));
        }
    }

    return resultImage;
}
