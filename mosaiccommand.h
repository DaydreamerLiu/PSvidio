#ifndef MOSAICCOMMAND_H
#define MOSAICCOMMAND_H

#include "imagecommand.h"
#include <QRect>

class MosaicCommand : public ImageCommand
{
public:
    MosaicCommand(const QImage &originalImage, const QRect &region, int blockSize = 10);
    QImage execute() override;
    
    QRect region() const;
    int blockSize() const;

private:
    QRect m_region;    // 马赛克区域
    int m_blockSize;   // 马赛克块大小
};

#endif // MOSAICCOMMAND_H