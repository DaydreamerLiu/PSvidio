#include "mosaiccommand.h"

MosaicCommand::MosaicCommand(const QImage &originalImage, const QRect &region, int blockSize)
    : ImageCommand(originalImage, "局部马赛克"), m_region(region), m_blockSize(blockSize)
{
}

QImage MosaicCommand::execute()
{
    QImage resultImage = m_originalImage.copy();
    
    // 确保马赛克区域在图像范围内
    QRect validRegion = m_region.intersected(QRect(0, 0, resultImage.width(), resultImage.height()));
    
    if (validRegion.isEmpty() || m_blockSize <= 1) {
        return resultImage;
    }
    
    // 确保图像格式为RGB32以便直接操作像素数据
    if (resultImage.format() != QImage::Format_RGB32) {
        resultImage = resultImage.convertToFormat(QImage::Format_RGB32);
    }
    
    // 遍历马赛克区域，按块处理
    for (int y = validRegion.top(); y < validRegion.bottom(); y += m_blockSize) {
        for (int x = validRegion.left(); x < validRegion.right(); x += m_blockSize) {
            // 计算当前块的边界
            int blockWidth = qMin(m_blockSize, validRegion.right() - x);
            int blockHeight = qMin(m_blockSize, validRegion.bottom() - y);
            
            // 计算块内像素的平均颜色
            int totalR = 0, totalG = 0, totalB = 0, totalCount = 0;
            
            // 遍历块内像素，计算颜色总和
            for (int blockY = 0; blockY < blockHeight; ++blockY) {
                QRgb *line = reinterpret_cast<QRgb*>(resultImage.scanLine(y + blockY));
                for (int blockX = 0; blockX < blockWidth; ++blockX) {
                    QRgb pixel = line[x + blockX];
                    totalR += qRed(pixel);
                    totalG += qGreen(pixel);
                    totalB += qBlue(pixel);
                    totalCount++;
                }
            }
            
            if (totalCount > 0) {
                int avgR = totalR / totalCount;
                int avgG = totalG / totalCount;
                int avgB = totalB / totalCount;
                QRgb averageColor = qRgb(avgR, avgG, avgB);
                
                // 将整个块设置为平均颜色
                for (int blockY = 0; blockY < blockHeight; ++blockY) {
                    QRgb *line = reinterpret_cast<QRgb*>(resultImage.scanLine(y + blockY));
                    for (int blockX = 0; blockX < blockWidth; ++blockX) {
                        line[x + blockX] = averageColor;
                    }
                }
            }
        }
    }
    
    return resultImage;
}

QRect MosaicCommand::region() const
{
    return m_region;
}

int MosaicCommand::blockSize() const
{
    return m_blockSize;
}