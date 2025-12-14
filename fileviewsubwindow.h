#ifndef FILEVIEWSUBWINDOW_H
#define FILEVIEWSUBWINDOW_H

#include <QMdiSubWindow>
#include <QWidget>
#include <QLabel>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QAudioOutput>

class FileViewSubWindow final : public QMdiSubWindow
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FileViewSubWindow)  // 禁用拷贝/移动，符合Qt6最佳实践

public:
    // 构造函数：explicit避免隐式转换，QWidget* parent = nullptr符合Qt6默认参数规范
    explicit FileViewSubWindow(const QString &filePath, QWidget *parent = nullptr);
    ~FileViewSubWindow() override = default;  // 显式默认析构，符合现代C++

private:
    // 加载媒体文件的私有方法
    void loadImage(const QString &filePath);  // 加载图片（JPG/PNG/BMP）
    void loadVideo(const QString &filePath);  // 加载视频（MP4/AVI/MOV）

    // 成员变量：使用前向声明+初始化，遵循Qt6内存管理（父子机制）
    QWidget *m_contentWidget = nullptr;
    QLabel *m_imageLabel = nullptr;
    QMediaPlayer *m_mediaPlayer = nullptr;
    QVideoWidget *m_videoWidget = nullptr;
    QAudioOutput *m_audioOutput = nullptr;   // Qt6 音频输出（替代原setVolume）
};

#endif // FILEVIEWSUBWINDOW_H
