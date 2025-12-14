#include "fileviewsubwindow.h"
#include <QPixmap>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QSizePolicy>
#include <QDebug>

// Qt6.9.2 构造函数
FileViewSubWindow::FileViewSubWindow(const QString &filePath, QWidget *parent)
    : QMdiSubWindow(parent)
{
    // 初始化内容容器
    m_contentWidget = new QWidget(this);
    setWidget(m_contentWidget);

    // 布局配置（零边距）
    QVBoxLayout *mainLayout = new QVBoxLayout(m_contentWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 空文件路径处理
    if (filePath.isEmpty()) {
        setWindowTitle(tr("新建空白窗口"));
        return;
    }

    // 设置窗口标题
    setWindowTitle(QFileInfo(filePath).fileName());

    // 媒体类型判断
    const QString suffix = QFileInfo(filePath).suffix().toLower();
    if (suffix == "jpg" || suffix == "jpeg" || suffix == "png" || suffix == "bmp") {
        loadImage(filePath);
    } else if (suffix == "mp4" || suffix == "avi" || suffix == "mov") {
        loadVideo(filePath);
    } else {
        setWindowTitle(tr("不支持的文件类型：%1").arg(suffix));
    }
}

// 加载图片（无修改，Qt6.9.2 兼容）
void FileViewSubWindow::loadImage(const QString &filePath)
{
    m_imageLabel = new QLabel(this);
    QPixmap pixmap(filePath);

    m_imageLabel->setPixmap(pixmap.scaled(m_imageLabel->size(),
                                          Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation));
    m_imageLabel->setScaledContents(true);
    m_imageLabel->setAlignment(Qt::AlignCenter);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(m_imageLabel);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    m_contentWidget->layout()->addWidget(scrollArea);
}

// 加载视频（Qt6.9.2 最新API：使用QAudioOutput控制音量）
void FileViewSubWindow::loadVideo(const QString &filePath)
{
    // 1. 初始化媒体播放器和音频输出（Qt6 必须绑定音频输出）
    m_mediaPlayer = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);  // Qt6 音频输出实例
    m_mediaPlayer->setAudioOutput(m_audioOutput);  // 绑定音频输出到播放器

    // 2. 初始化视频显示控件
    m_videoWidget = new QVideoWidget(this);
    m_mediaPlayer->setVideoOutput(m_videoWidget);

    // 3. 设置媒体源（Qt6 替代原setMedia）
    m_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));

    // 4. 视频窗口配置
    m_videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_videoWidget->setAspectRatioMode(Qt::KeepAspectRatio);
    m_contentWidget->layout()->addWidget(m_videoWidget);

    // 5. Qt6 音量控制（通过QAudioOutput设置，范围0.0~1.0）
    m_audioOutput->setVolume(0.5);  // 50% 音量（替代原setVolume(50)）

    // 6. 自动播放
    m_mediaPlayer->play();

    // 调试信息（可选）
    qDebug() << "视频加载成功：" << filePath;
}
