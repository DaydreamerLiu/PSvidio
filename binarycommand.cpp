#include "binarycommand.h"

BinaryCommand::BinaryCommand(const QImage &originalImage, int threshold)
    : ImageCommand(originalImage, "二值化"), m_threshold(threshold)
{
}

QImage BinaryCommand::execute()
{
    QImage resultImage = m_originalImage.copy();

    // 遍历每个像素
    for (int y = 0; y < resultImage.height(); ++y) {
        for (int x = 0; x < resultImage.width(); ++x) {
            QColor pixelColor = resultImage.pixelColor(x, y);
            // 计算灰度值
            int grayValue = (pixelColor.red() + pixelColor.green() + pixelColor.blue()) / 3;
            // 根据阈值二值化
            if (grayValue > m_threshold) {
                resultImage.setPixelColor(x, y, Qt::white);
            } else {
                resultImage.setPixelColor(x, y, Qt::black);
            }
        }
    }

    return resultImage;
}

int BinaryCommand::threshold() const
{
    return m_threshold;
}
