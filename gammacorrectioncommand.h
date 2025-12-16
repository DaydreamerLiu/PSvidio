#ifndef GAMMACORRECTIONCOMMAND_H
#define GAMMACORRECTIONCOMMAND_H

#include "imagecommand.h"

class GammaCorrectionCommand : public ImageCommand
{
public:
    GammaCorrectionCommand(const QImage &originalImage, double gamma);
    QImage execute() override;
    double gamma() const;

private:
    double m_gamma;
};

#endif // GAMMACORRECTIONCOMMAND_H