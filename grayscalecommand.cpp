#include "grayscalecommand.h"

GrayscaleCommand::GrayscaleCommand(const QImage &originalImage)
    : ImageCommand(originalImage, "灰度化")
{
}

QImage GrayscaleCommand::execute()
{
    QImage resultImage = m_inputImage.copy();

    // 遍历每个像素
    for (int y = 0; y < resultImage.height(); ++y) {
        for (int x = 0; x < resultImage.width(); ++x) {
            QColor pixelColor = resultImage.pixelColor(x, y);
            // 计算灰度值：(R+G+B)/3
            int grayValue = (pixelColor.red() + pixelColor.green() + pixelColor.blue()) / 3;
            // 设置灰度像素
            resultImage.setPixelColor(x, y, QColor(grayValue, grayValue, grayValue));
        }
    }

    return resultImage;
}
