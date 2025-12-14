#include "fileviewsubwindow.h"
#include <QPixmap>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QSizePolicy>
#include <QDebug>
#include <QResizeEvent>



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

// 核心：加载图片（保持原始比例，初始适配窗口）
void FileViewSubWindow::loadImage(const QString &filePath)
{
    // 1. 加载原始图片（保存到成员变量，避免多次缩放失真）
    m_originalImage = QImage(filePath);
    if (m_originalImage.isNull()) {
        m_imageLabel = new QLabel(tr("图片加载失败：%1").arg(filePath), this);
        m_imageLabel->setAlignment(Qt::AlignCenter);
        m_contentWidget->layout()->addWidget(m_imageLabel);
        return;
    }

    // 2. 初始化图片显示标签
    m_imageLabel = new QLabel(this);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    // 3. 滚动区域配置（原生适配高DPI）
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(m_imageLabel);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_contentWidget->layout()->addWidget(scrollArea);

    // 4. 初始缩放：适配窗口（图片最大边不超过窗口，保持比例）
    QSize windowSize = this->size() * 0.9;  // 留10%边距
    QSize imageSize = m_originalImage.size();
    imageSize.scale(windowSize, Qt::KeepAspectRatio);  // 按窗口适配比例
    m_scalePercent = qRound((imageSize.width() * 100.0) / m_originalImage.width());
    updateImageDisplay();
}

// 核心：更新图片显示（保持比例，按当前缩放比例渲染）
void FileViewSubWindow::updateImageDisplay()
{
    if (m_originalImage.isNull() || !m_imageLabel) return;

    // 计算缩放后的尺寸（保持原始比例）
    qreal scale = m_scalePercent / 100.0;
    QSize scaledSize = m_originalImage.size() * scale;

    // 高质量缩放（Qt6最优算法，无模糊）
    QPixmap scaledPixmap = QPixmap::fromImage(m_originalImage.scaled(
        scaledSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        ));

    // 显示图片（居中，不拉伸）
    m_imageLabel->setPixmap(scaledPixmap);
    m_imageLabel->adjustSize();  // 适配图片尺寸
}

// 对外接口：设置缩放比例（1~500%）
void FileViewSubWindow::setScaleFactor(int percent)
{
    // 限制范围：1% ~ 500%
    m_scalePercent = qBound(1, percent, 500);
    updateImageDisplay();
}

// 获取当前缩放比例（%）
int FileViewSubWindow::currentScalePercent() const
{
    return m_scalePercent;
}

// 重写滚轮事件：鼠标滚轮缩放（向上=放大，向下=缩小）
void FileViewSubWindow::wheelEvent(QWheelEvent *event)
{
    // 仅图片模式下响应滚轮
    if (m_originalImage.isNull()) {
        QMdiSubWindow::wheelEvent(event);
        return;
    }

    // 滚轮每步调整5%
    int delta = event->angleDelta().y() > 0 ? 5 : -5;
    int newPercent = m_scalePercent + delta;
    newPercent = qBound(1, newPercent, 500);  // 限制范围

    // 更新缩放并通知主窗口Slider同步
    m_scalePercent = newPercent;
    updateImageDisplay();
    emit scaleChanged(m_scalePercent);  // 触发主窗口更新Slider（QMdiSubWindow内置信号）
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
