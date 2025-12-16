#ifndef EDGEDETECTIONCOMMAND_H
#define EDGEDETECTIONCOMMAND_H

#include "imagecommand.h"

class EdgeDetectionCommand : public ImageCommand
{
public:
    EdgeDetectionCommand(const QImage &originalImage, int threshold = 50);
    QImage execute() override;
    
    // 获取当前阈值
    int threshold() const;

private:
    // Sobel边缘检测算法
    QImage sobelEdgeDetection(const QImage &image, int threshold);
    // 转换为灰度图
    QImage toGrayscale(const QImage &image);
    
    int m_threshold; // 边缘检测阈值
};

#endif // EDGEDETECTIONCOMMAND_H