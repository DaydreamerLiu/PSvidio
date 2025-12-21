#ifndef IMAGECOMMAND_H
#define IMAGECOMMAND_H

#include <QImage>
#include <QString>

class ImageCommand
{
public:
    ImageCommand(const QImage &originalImage, const QString &name);
    virtual ~ImageCommand() = default;

    // 设置输入图像（用于视频帧处理）
    void setInputImage(const QImage &image);
    // 执行命令
    virtual QImage execute() = 0;
    // 撤销命令
    QImage undo() const;
    // 获取命令名称
    QString name() const;

protected:
    QImage m_originalImage;    // 原始图像（用于撤销）
    QImage m_inputImage;       // 当前输入图像（用于执行）
    QString m_name;
};

#endif // IMAGECOMMAND_H