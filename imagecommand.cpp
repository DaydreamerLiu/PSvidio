#include "imagecommand.h"

ImageCommand::ImageCommand(const QImage &originalImage, const QString &name)
    : m_originalImage(originalImage), m_inputImage(originalImage), m_name(name)
{
}

void ImageCommand::setInputImage(const QImage &image)
{
    m_inputImage = image;
}

QImage ImageCommand::undo() const
{
    return m_originalImage;
}

QString ImageCommand::name() const
{
    return m_name;
}
