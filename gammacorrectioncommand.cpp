#include "gammacorrectioncommand.h"
#include <cmath>

GammaCorrectionCommand::GammaCorrectionCommand(const QImage &originalImage, double gamma)
    : ImageCommand(originalImage, "伽马变换"), m_gamma(gamma)
{
}

QImage GammaCorrectionCommand::execute()
{
    QImage resultImage = m_originalImage.copy();
    int width = resultImage.width();
    int height = resultImage.height();

    // 确保图像格式为RGB32以便直接操作像素数据
    if (resultImage.format() != QImage::Format_RGB32) {
        resultImage = resultImage.convertToFormat(QImage::Format_RGB32);
    }

    // 预计算伽马变换表，避免重复计算pow()函数
    int gammaTable[256];
    for (int i = 0; i < 256; ++i) {
        double normalized = i / 255.0;
        double gammaValue = pow(normalized, m_gamma);
        gammaTable[i] = qBound(0, qRound(gammaValue * 255), 255);
    }

    // 伽马变换
    for (int y = 0; y < height; ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(resultImage.scanLine(y));
        for (int x = 0; x < width; ++x) {
            QRgb pixel = line[x];

            // 使用预计算的变换表对每个颜色通道应用伽马变换
            int r = gammaTable[qRed(pixel)];
            int g = gammaTable[qGreen(pixel)];
            int b = gammaTable[qBlue(pixel)];

            // 设置变换后的像素
            line[x] = qRgb(r, g, b);
        }
    }

    return resultImage;
}

double GammaCorrectionCommand::gamma() const
{
    return m_gamma;
}
