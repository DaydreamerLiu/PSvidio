#include "binarycommand.h"

BinaryCommand::BinaryCommand(const QImage &originalImage, int threshold)
    : ImageCommand(originalImage, "二值化"), m_threshold(threshold)
{
}

QImage BinaryCommand::execute()
{
    QImage resultImage = m_originalImage.copy();
    int width = resultImage.width();
    int height = resultImage.height();

    // 确保图像格式为RGB32以便直接操作像素数据
    if (resultImage.format() != QImage::Format_RGB32) {
        resultImage = resultImage.convertToFormat(QImage::Format_RGB32);
    }

    // 遍历每个像素行
    for (int y = 0; y < height; ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(resultImage.scanLine(y));
        for (int x = 0; x < width; ++x) {
            // 获取像素值
            QRgb pixel = line[x];
            
            // 计算灰度值（使用更高效的加权平均法，考虑人眼对不同颜色的敏感度）
            int grayValue = qGray(pixel);
            
            // 根据阈值二值化
            line[x] = (grayValue > m_threshold) ? qRgb(255, 255, 255) : qRgb(0, 0, 0);
        }
    }

    return resultImage;
}

int BinaryCommand::threshold() const
{
    return m_threshold;
}
