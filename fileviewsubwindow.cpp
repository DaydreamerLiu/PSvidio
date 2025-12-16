#include "fileviewsubwindow.h"
#include <QPixmap>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QSizePolicy>
#include <QDebug>
#include <QResizeEvent>
#include <QStyle>


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
    // 1. 加载原始图片（保存到成员变量）
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

    // 4. 初始化当前图片和命令历史
    m_currentImage = m_originalImage;
    m_commandHistory.clear();
    m_historyIndex = -1;

    // 5. 初始缩放：适配窗口（图片最大边不超过窗口，保持比例）
    QSize windowSize = this->size() * 20;  // 留10%边距
    qDebug() << "this->size()" << this->size();
    qDebug() << "初始比例：" << windowSize;
    QSize imageSize = m_currentImage.size();
    imageSize.scale(windowSize, Qt::KeepAspectRatio);  // 按窗口适配比例
    m_scalePercent = qRound((imageSize.width() * 100.0) / m_currentImage.width());
    updateImageDisplay();
}

// 核心：更新图片显示（保持比例，按当前缩放比例渲染）
void FileViewSubWindow::updateImageDisplay()
{
    if (m_currentImage.isNull() || !m_imageLabel) return;

    // 计算缩放后的尺寸（保持原始比例）
    qreal scale = m_scalePercent / 100.0;
    QSize scaledSize = m_currentImage.size() * scale;

    // 高质量缩放（Qt6最优算法，无模糊）
    QPixmap scaledPixmap = QPixmap::fromImage(m_currentImage.scaled(
        scaledSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        ));

    // 显示图片（居中，不拉伸）
    m_imageLabel->setPixmap(scaledPixmap);
    m_imageLabel->adjustSize();  // 适配图片尺寸
}

// 应用图像处理命令
void FileViewSubWindow::applyImageCommand(ImageCommand *command)
{
    if (!command || m_currentImage.isNull()) return;

    // 清除当前历史记录之后的命令
    while (m_historyIndex < m_commandHistory.size() - 1) {
        delete m_commandHistory.takeLast();
    }

    // 添加新命令到历史记录
    m_commandHistory.append(command);
    m_historyIndex++;

    // 执行命令并更新当前图片
    m_currentImage = command->execute();
    updateImageDisplay();
    
    // 发出命令应用信号
    emit commandApplied(command);
}

// 检查是否可以撤销
bool FileViewSubWindow::canUndo() const
{
    return m_historyIndex >= 0;
}

// 检查是否可以重做
bool FileViewSubWindow::canRedo() const
{
    return m_historyIndex < m_commandHistory.size() - 1;
}

// 撤销操作
void FileViewSubWindow::undo()
{
    if (!canUndo()) return;

    m_historyIndex--;

    // 如果没有历史记录，显示原始图片
    if (m_historyIndex < 0) {
        m_currentImage = m_originalImage;
        emit commandApplied(nullptr); // 没有当前命令
    } else {
        // 否则，显示上一个命令执行前的图片
        m_currentImage = m_commandHistory[m_historyIndex]->undo();
        emit commandApplied(m_historyIndex >= 0 ? m_commandHistory[m_historyIndex] : nullptr);
    }

    updateImageDisplay();
}

// 获取当前图像
QImage FileViewSubWindow::getCurrentImage() const
{
    return m_currentImage;
}

// 获取当前应用的命令
ImageCommand* FileViewSubWindow::getCurrentCommand() const
{
    if (m_historyIndex >= 0 && m_historyIndex < m_commandHistory.size()) {
        return m_commandHistory[m_historyIndex];
    }
    return nullptr;
}

// 重做操作
void FileViewSubWindow::redo()
{
    if (!canRedo()) return;

    m_historyIndex++;

    // 执行下一个命令
    m_currentImage = m_commandHistory[m_historyIndex]->execute();
    updateImageDisplay();
    
    // 发出命令应用信号
    emit commandApplied(m_commandHistory[m_historyIndex]);
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
// 格式化时间：毫秒 → 分:秒（如 123000ms → 2:03）
QString FileViewSubWindow::formatTime(qint64 ms)
{
    int seconds = ms / 1000;
    int minutes = seconds / 60;
    seconds = seconds % 60;
    return QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
}
// 加载视频（Qt6.9.2 最新API：使用QAudioOutput控制音量）
void FileViewSubWindow::loadVideo(const QString &filePath)
{
    // 1. 初始化媒体播放器和音频输出（Qt6 标准）
    m_mediaPlayer = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_mediaPlayer->setAudioOutput(m_audioOutput);

    // 2. 初始化视频显示控件
    m_videoWidget = new QVideoWidget(this);
    m_videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_videoWidget->setAspectRatioMode(Qt::KeepAspectRatio); // 保持视频比例
    m_mediaPlayer->setVideoOutput(m_videoWidget);

    // 3. 创建视频控制栏（水平布局）
    QWidget *controlBar = new QWidget(this);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlBar);
    controlLayout->setContentsMargins(15, 10, 15, 10); // 增大边距
    controlLayout->setSpacing(20); // 增大控件间距

    // 3.1 播放/暂停按钮（带图标，Qt6 原生样式）
    m_btnPlayPause = new QPushButton(this);
    m_btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay)); // 初始播放图标
    m_btnPlayPause->setIconSize(QSize(24, 24));
    m_btnPlayPause->setFixedSize(36, 36);
    controlLayout->addWidget(m_btnPlayPause);

    // 3.2 音量控制（标签+滑块）
    QLabel *volLabel = new QLabel(tr("音量："), this);
    m_sliderVolume = new QSlider(Qt::Horizontal, this);
    m_sliderVolume->setRange(0, 100); // 音量范围 0~100（映射到 0.0~1.0）
    m_sliderVolume->setValue(50);     // 默认50%音量
    m_sliderVolume->setFixedWidth(120);
    controlLayout->addWidget(volLabel);
    controlLayout->addWidget(m_sliderVolume);

    // 3.3 进度条（保持Expanding策略，占满剩余空间）
    m_sliderProgress = new QSlider(Qt::Horizontal, this);
    m_sliderProgress->setRange(0, 100);
    m_sliderProgress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    controlLayout->addWidget(m_sliderProgress);

    // 3.4 时间显示（增大标签宽度）
    m_labelTime = new QLabel("0:00/0:00", this);
    m_labelTime->setFixedWidth(100); // 避免时间文字拥挤
    m_labelTime->setAlignment(Qt::AlignCenter);
    controlLayout->addWidget(m_labelTime);

    // 4. 复用构造函数的布局，添加视频控件和控制栏
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(m_contentWidget->layout());
    if (mainLayout) {
        mainLayout->addWidget(m_videoWidget); // 视频控件占满上方空间
        mainLayout->addWidget(controlBar);    // 控制栏在下方
    }

    // 5. 设置媒体源（Qt6 标准）
    m_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));

    // 6. 关联信号槽（核心控制逻辑）
    // 播放/暂停按钮
    connect(m_btnPlayPause, &QPushButton::clicked, this, &FileViewSubWindow::onPlayPauseClicked);
    // 音量滑块
    connect(m_sliderVolume, &QSlider::valueChanged, this, &FileViewSubWindow::onVolumeSliderChanged);
    // 进度条（拖动时标记，释放时更新位置，避免实时卡顿）
    connect(m_sliderProgress, &QSlider::sliderPressed, this, [=]() { m_isProgressDragging = true; });
    connect(m_sliderProgress, &QSlider::sliderReleased, this, &FileViewSubWindow::onProgressSliderReleased);
    connect(m_sliderProgress, &QSlider::valueChanged, this, &FileViewSubWindow::onProgressSliderChanged);
    // 播放器状态变化（更新按钮图标）
    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged, this, &FileViewSubWindow::onPlayerStateChanged);
    // 视频时长加载完成（更新进度条范围）
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &FileViewSubWindow::onDurationChanged);
    // 播放位置变化（更新进度条和时间显示）
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &FileViewSubWindow::onPositionChanged);

    // 7. 初始音量设置
    m_audioOutput->setVolume(0.5); // 50% 音量（Qt6 范围 0.0~1.0）

    // 8. 自动播放
    m_mediaPlayer->play();

    qDebug() << "视频加载成功：" << filePath;
}

// ========== 视频控制槽函数实现 ==========
// 播放/暂停切换
void FileViewSubWindow::onPlayPauseClicked()
{
    if (!m_mediaPlayer) return;

    if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        m_mediaPlayer->pause(); // 暂停
    } else {
        m_mediaPlayer->play();  // 播放
    }
}

// 音量调节（滑块值 0~100 → 映射到 0.0~1.0）
void FileViewSubWindow::onVolumeSliderChanged(int value)
{
    if (!m_audioOutput) return;
    m_audioOutput->setVolume(value / 100.0); // Qt6 音量范围 0.0~1.0
}

// 进度条拖动（仅记录值，释放时更新）
void FileViewSubWindow::onProgressSliderChanged(int value)
{
    if (!m_mediaPlayer || m_isProgressDragging) return; // 拖动时不实时更新，避免卡顿
}

// 进度条释放（更新视频播放位置）
void FileViewSubWindow::onProgressSliderReleased()
{
    if (!m_mediaPlayer) return;

    m_isProgressDragging = false;
    qint64 duration = m_mediaPlayer->duration();
    if (duration <= 0) return;

    // 计算目标位置（进度条值 → 毫秒）
    qint64 targetPos = (m_sliderProgress->value() * duration) / 100;
    m_mediaPlayer->setPosition(targetPos); // 更新播放位置
}

// 播放状态变化（更新按钮图标）
void FileViewSubWindow::onPlayerStateChanged(QMediaPlayer::PlaybackState state)
{
    if (!m_btnPlayPause) return;

    if (state == QMediaPlayer::PlayingState) {
        // 播放中 → 显示暂停图标
        m_btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    } else {
        // 暂停/停止 → 显示播放图标
        m_btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    }
}

// 视频时长加载完成（更新进度条范围）
void FileViewSubWindow::onDurationChanged(qint64 duration)
{
    if (!m_sliderProgress || !m_labelTime) return;

    // 进度条范围设为 0~100（百分比），方便计算
    m_sliderProgress->setRange(0, 100);
    // 更新总时长显示
    m_labelTime->setText(QString("%1/%2").arg("0:00").arg(formatTime(duration)));
}

// 播放位置变化（更新进度条和时间显示）
void FileViewSubWindow::onPositionChanged(qint64 position)
{
    if (!m_sliderProgress || !m_labelTime || m_isProgressDragging) return;

    qint64 duration = m_mediaPlayer->duration();
    if (duration <= 0) return;

    // 计算进度百分比（0~100）
    int progress = qRound((position * 100.0) / duration);
    m_sliderProgress->setValue(progress);

    // 更新时间显示（当前/总时长）
    m_labelTime->setText(QString("%1/%2").arg(formatTime(position)).arg(formatTime(duration)));
}
