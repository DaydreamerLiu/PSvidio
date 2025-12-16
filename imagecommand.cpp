#include "imagecommand.h"

ImageCommand::ImageCommand(const QImage &originalImage, const QString &name)
    : m_originalImage(originalImage), m_name(name)
{
}

QImage ImageCommand::undo() const
{
    return m_originalImage;
}

QString ImageCommand::name() const
{
    return m_name;
}
