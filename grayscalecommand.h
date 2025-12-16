#ifndef GRAYSCALECOMMAND_H
#define GRAYSCALECOMMAND_H

#include "imagecommand.h"

class GrayscaleCommand : public ImageCommand
{
public:
    explicit GrayscaleCommand(const QImage &originalImage);
    QImage execute() override;
};

#endif // GRAYSCALECOMMAND_H