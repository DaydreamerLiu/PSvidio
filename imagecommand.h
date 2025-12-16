#ifndef IMAGECOMMAND_H
#define IMAGECOMMAND_H

#include <QImage>
#include <QString>

class ImageCommand
{
public:
    ImageCommand(const QImage &originalImage, const QString &name);
    virtual ~ImageCommand() = default;

    // 执行命令
    virtual QImage execute() = 0;
    // 撤销命令
    QImage undo() const;
    // 获取命令名称
    QString name() const;

protected:
    QImage m_originalImage;
    QString m_name;
};

#endif // IMAGECOMMAND_H