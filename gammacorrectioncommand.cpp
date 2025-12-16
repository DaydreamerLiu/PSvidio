#include "gammacorrectioncommand.h"
#include <cmath>

GammaCorrectionCommand::GammaCorrectionCommand(const QImage &originalImage, double gamma)
    : ImageCommand(originalImage, "伽马变换"), m_gamma(gamma)
{
}

QImage GammaCorrectionCommand::execute()
{
    QImage resultImage = m_originalImage.copy();

    // 伽马变换
    for (int y = 0; y < resultImage.height(); ++y) {
        for (int x = 0; x < resultImage.width(); ++x) {
            QColor pixelColor = resultImage.pixelColor(x, y);

            // 对每个颜色通道应用伽马变换
            int r = qRound(pow(pixelColor.red() / 255.0, m_gamma) * 255);
            int g = qRound(pow(pixelColor.green() / 255.0, m_gamma) * 255);
            int b = qRound(pow(pixelColor.blue() / 255.0, m_gamma) * 255);

            // 确保值在0-255范围内
            r = qBound(0, r, 255);
            g = qBound(0, g, 255);
            b = qBound(0, b, 255);

            // 设置变换后的像素
            resultImage.setPixelColor(x, y, QColor(r, g, b));
        }
    }

    return resultImage;
}

double GammaCorrectionCommand::gamma() const
{
    return m_gamma;
}
