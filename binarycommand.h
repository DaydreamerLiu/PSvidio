#ifndef BINARYCOMMAND_H
#define BINARYCOMMAND_H

#include "imagecommand.h"

class BinaryCommand : public ImageCommand
{
public:
    BinaryCommand(const QImage &originalImage, int threshold);
    QImage execute() override;
    int threshold() const;

private:
    int m_threshold;
};

#endif // BINARYCOMMAND_H