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

    // 确保图像格式为RGB32以便直接操作像素数据
    if (resultImage.format() != QImage::Format_RGB32) {
        resultImage = resultImage.convertToFormat(QImage::Format_RGB32);
    }

    // 获取原始图像的像素数据（同样转换为RGB32格式）
    QImage originalRgb = m_originalImage.convertToFormat(QImage::Format_RGB32);

    // 3×3均值滤波
    for (int y = 0; y < height; ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(resultImage.scanLine(y));
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
                        QRgb pixel = originalRgb.pixel(nx, ny);
                        sumR += qRed(pixel);
                        sumG += qGreen(pixel);
                        sumB += qBlue(pixel);
                        count++;
                    }
                }
            }

            // 计算平均值
            int avgR = sumR / count;
            int avgG = sumG / count;
            int avgB = sumB / count;

            // 设置滤波后的像素
            line[x] = qRgb(avgR, avgG, avgB);
        }
    }

    return resultImage;
}
