#ifndef MEANFILTERCOMMAND_H
#define MEANFILTERCOMMAND_H

#include "imagecommand.h"

class MeanFilterCommand : public ImageCommand
{
public:
    explicit MeanFilterCommand(const QImage &originalImage);
    QImage execute() override;
};

#endif // MEANFILTERCOMMAND_H