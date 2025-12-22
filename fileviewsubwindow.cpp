#include "fileviewsubwindow.h"
#include <QPixmap>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QSizePolicy>
#include <QDebug>
#include <QResizeEvent>
#include <QStyle>
#include <QDir>
#include <QTimer>
#include <QEventLoop>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QPainter>


// Qt6.9.2 构造函数
FileViewSubWindow::FileViewSubWindow(const QString &filePath, QWidget *parent)
    : QMdiSubWindow(parent)
    , m_filePath(filePath)
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

// 应用图像处理命令到视频
void FileViewSubWindow::applyImageCommandToVideo(ImageCommand *command)
{
    if (!command || !isVideoFile()) return;
    
    // 清除当前历史记录之后的命令
    while (m_historyIndex < m_commandHistory.size() - 1) {
        delete m_commandHistory.takeLast();
    }
    
    // 添加新命令到历史记录
    m_commandHistory.append(command);
    m_historyIndex++;
    
    // 启用视频处理
    m_isProcessingEnabled = true;
    
    // 通知用户视频处理命令已应用
    qDebug() << "视频处理命令已应用到历史记录，视频将实时显示处理效果";
    
    // 发出命令应用信号
    emit commandApplied(command);
}

// 视频帧变化处理槽函数
void FileViewSubWindow::onVideoFrameChanged(const QVideoFrame &frame) {
    if (!m_isVideoMode || !m_isProcessingEnabled || !frame.isValid()) return;
    
    // 复制视频帧并转换为可读写格式
    QVideoFrame videoFrame = frame;
    if (!videoFrame.map(QVideoFrame::ReadOnly)) return;
    
    // 将QVideoFrame转换为QImage
    QImage currentFrame;
    
    // 使用Qt 6的toImage()方法直接转换，更简单可靠
    currentFrame = videoFrame.toImage();
    
    // 确保格式转换正确
    if (currentFrame.format() == QImage::Format_Invalid) {
        videoFrame.unmap();
        return;
    }
    
    // 如果不是RGB888格式，转换为RGB888
    if (currentFrame.format() != QImage::Format_RGB888) {
        currentFrame = currentFrame.convertToFormat(QImage::Format_RGB888);
    }
    
    // 取消映射
    videoFrame.unmap();
    
    // 保存原始帧
    m_originalVideoFrame = currentFrame;
    
    // 应用所有图像处理命令
    QImage processedFrame = currentFrame;
    for (int i = 0; i <= m_historyIndex; i++) {
        if (m_commandHistory[i]) {
            // 设置当前帧作为命令的输入图像
            m_commandHistory[i]->setInputImage(processedFrame);
            // 执行命令并获取处理后的帧
            processedFrame = m_commandHistory[i]->execute();
            
            // 确保处理后的帧格式仍然是RGB888
            if (processedFrame.format() != QImage::Format_RGB888) {
                processedFrame = processedFrame.convertToFormat(QImage::Format_RGB888);
            }
        }
    }
    
    // 更新视频显示
    m_currentVideoFrame = processedFrame;
    
    // 在QGraphicsView上显示处理后的视频帧
    if (m_graphicsScene && m_graphicsView && m_videoPixmapItem) {
        // 创建Pixmap并调整大小以适应视图
        QPixmap processedPixmap = QPixmap::fromImage(processedFrame);
        if (!processedPixmap.isNull()) {
            // 调整Pixmap大小以适应视图
            QSize viewSize = m_graphicsView->size();
            QPixmap scaledPixmap = processedPixmap.scaled(viewSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            
            // 更新PixmapItem
            m_videoPixmapItem->setPixmap(scaledPixmap);
            
            // 调整场景大小以匹配视图
            m_graphicsScene->setSceneRect(0, 0, scaledPixmap.width(), scaledPixmap.height());
        }
    }
}

// 视频帧定时处理槽函数
void FileViewSubWindow::onVideoFrameTimerTimeout()
{
    // 该函数现在已经被onVideoFrameChanged替代
    return;
}

// 更新视频显示
void FileViewSubWindow::updateVideoDisplay()
{
    if (!m_graphicsView || !m_videoPixmapItem || m_currentVideoFrame.isNull()) return;
    
    // 计算缩放后的尺寸（保持原始比例）
    qreal scale = m_scalePercent / 100.0;
    QSize scaledSize = m_currentVideoFrame.size() * scale;
    
    // 高质量缩放
    QPixmap scaledPixmap = QPixmap::fromImage(m_currentVideoFrame.scaled(
        scaledSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        ));
    
    // 显示处理后的视频帧
    m_videoPixmapItem->setPixmap(scaledPixmap);
    m_graphicsScene->setSceneRect(0, 0, scaledPixmap.width(), scaledPixmap.height());
}

// 保存处理后的视频
bool FileViewSubWindow::saveProcessedVideo(const QString &filePath)
{
    if (!m_mediaPlayer || !m_isVideoMode || m_commandHistory.isEmpty() || m_historyIndex < 0) {
        return false;
    }
    
    // 暂停视频播放
    bool wasPlaying = m_isPlaying;
    if (wasPlaying) {
        m_mediaPlayer->pause();
    }
    
    // 显示保存进度信息
    qDebug() << "开始保存处理后的视频：" << filePath;
    qDebug() << "视频尺寸：" << m_videoWidth << "x" << m_videoHeight << "，时长：" << m_videoDuration << "ms";
    
    // 创建输出目录
    QDir outputDir(filePath);
    if (!outputDir.exists()) {
        if (!outputDir.mkpath(filePath)) {
            return false;
        }
    }
    
    // 重置视频到开始位置
    m_mediaPlayer->setPosition(0);
    
    // 等待视频加载
    m_mediaPlayer->play();
    m_mediaPlayer->pause();
    
    // 处理并保存每一帧（简化版，仅保存当前帧）
    // 注意：完整的视频保存功能需要使用QMediaRecorder或其他库
    QPixmap pixmap = m_videoWidget->grab();
    if (pixmap.isNull()) {
        if (wasPlaying) {
            m_mediaPlayer->play();
        }
        return false;
    }
    
    QImage qImage = pixmap.toImage().convertToFormat(QImage::Format_RGB888);
    
    // 应用所有图像处理命令
    QImage processedFrame = qImage;
    for (int j = 0; j <= m_historyIndex; j++) {
        m_commandHistory[j]->setInputImage(processedFrame);
        processedFrame = m_commandHistory[j]->execute();
    }
    
    // 保存处理后的帧
    QString framePath = outputDir.path() + QString("/current_frame.png");
    bool saved = processedFrame.save(framePath);
    
    // 恢复视频状态
    if (wasPlaying) {
        m_mediaPlayer->play();
    }
    
    if (saved) {
        qDebug() << "视频帧保存完成：" << framePath;
    } else {
        qDebug() << "保存帧失败：" << framePath;
    }
    
    return saved;
}

// 应用图像处理命令
void FileViewSubWindow::applyImageCommand(ImageCommand *command)
{
    if (!command || m_currentImage.isNull()) return;

    // 清除当前历史记录之后的命令
    while (m_historyIndex < m_commandHistory.size() - 1) {
        delete m_commandHistory.takeLast();
    }

    // 添加新命令到历史记录（每次调整参数都记录为新命令，不替换相同类型的命令）
    m_commandHistory.append(command);
    m_historyIndex++;

    // 重新执行所有命令以确保状态正确
    m_currentImage = m_originalImage;
    for (int i = 0; i <= m_historyIndex; i++) {
        m_currentImage = m_commandHistory[i]->execute();
    }
    
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

    if (isVideoFile()) {
        // 视频模式下，更新处理状态
        if (m_historyIndex < 0) {
            // 如果没有历史记录，禁用视频处理
            m_isProcessingEnabled = false;
            if (m_frameCaptureTimer && m_frameCaptureTimer->isActive()) {
                m_frameCaptureTimer->stop();
            }
        }
        emit commandApplied(m_historyIndex >= 0 ? m_commandHistory[m_historyIndex] : nullptr);
    } else {
        // 图片模式下的处理
        if (m_historyIndex < 0) {
            m_currentImage = m_originalImage;
            emit commandApplied(nullptr); // 没有当前命令
        } else {
            // 重新执行从第一个命令到当前历史索引的所有命令
            m_currentImage = m_originalImage;
            for (int i = 0; i <= m_historyIndex; i++) {
                m_currentImage = m_commandHistory[i]->execute();
            }
            emit commandApplied(m_commandHistory[m_historyIndex]);
        }
        updateImageDisplay();
    }
}

// 检查是否为视频文件
bool FileViewSubWindow::isVideoFile() const
{
    return m_isVideoMode;
}

// 获取当前视频帧（仅视频模式）
QImage FileViewSubWindow::getCurrentVideoFrame() const
{
    if (!m_videoWidget || !m_isVideoMode) {
        return QImage();
    }
    
    // 从视频组件中捕获当前帧
    QPixmap pixmap = m_videoWidget->grab();
    if (pixmap.isNull()) {
        return QImage();
    }
    
    // 转换为QImage并返回
    QImage qImage = pixmap.toImage().convertToFormat(QImage::Format_RGB888);
    
    // 添加调试信息
    static int frameCount = 0;
    if (frameCount++ % 10 == 0) { // 每10帧输出一次信息
        qDebug() << "获取视频帧：宽=" << qImage.width() << "高=" << qImage.height() << "格式=" << qImage.format() << "是否为空=" << qImage.isNull();
    }
    
    return qImage;
}

// 获取当前图像（图片或视频帧）
QImage FileViewSubWindow::getCurrentImage() const
{
    if (isVideoFile()) {
        return getCurrentVideoFrame();
    } else {
        return m_currentImage;
    }
}

// 获取原始图像
QImage FileViewSubWindow::getOriginalImage() const
{
    if (isVideoFile()) {
        return m_originalVideoFrame;
    } else {
        return m_originalImage;
    }
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

    if (isVideoFile()) {
        // 视频模式下，确保处理已启用
        m_isProcessingEnabled = true;
        if (m_frameCaptureTimer && !m_frameCaptureTimer->isActive()) {
            m_frameCaptureTimer->start(33); // 约30fps
        }
        emit commandApplied(m_commandHistory[m_historyIndex]);
    } else {
        // 图片模式下的处理
        m_currentImage = m_originalImage;
        for (int i = 0; i <= m_historyIndex; i++) {
            m_currentImage = m_commandHistory[i]->execute();
        }
        
        updateImageDisplay();
        
        // 发出命令应用信号
        emit commandApplied(m_commandHistory[m_historyIndex]);
    }
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

// 获取文件路径
QString FileViewSubWindow::getFilePath() const
{
    return m_filePath;
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
// 加载视频（使用Qt多媒体框架）
void FileViewSubWindow::loadVideo(const QString &filePath)
{
    // 1. 检查文件是否存在
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qDebug() << "视频文件不存在：" << filePath;
        return;
    }
    
    if (!fileInfo.isReadable()) {
        qDebug() << "视频文件不可读（权限问题）：" << filePath;
        return;
    }
    
    qDebug() << "视频文件存在且可读：" << filePath;
    
    // 2. 初始化Qt多媒体组件
    m_mediaPlayer = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    
    // 创建一个隐藏的视频Widget用于捕获原始帧
    m_videoWidget = new QVideoWidget(this);
    m_videoWidget->setVisible(false);
    m_videoWidget->setMinimumSize(1, 1); // 确保视频Widget有最小尺寸，以便grab()能正常工作
    
    // 3. 设置媒体源
    m_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
    m_mediaPlayer->setVideoOutput(m_videoWidget);
    m_mediaPlayer->setAudioOutput(m_audioOutput);
    
    // 4. 创建QGraphicsView和相关组件用于显示处理后的视频帧
    m_graphicsScene = new QGraphicsScene(this);
    m_graphicsView = new QGraphicsView(m_graphicsScene, this);
    m_graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);
    m_graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_graphicsView->setFrameShape(QFrame::NoFrame);
    
    // 创建一个PixmapItem用于显示视频帧
    m_videoPixmapItem = new QGraphicsPixmapItem();
    m_graphicsScene->addItem(m_videoPixmapItem);
    
    // 4. 初始化视频控制栏（水平布局）
    QWidget *controlBar = new QWidget(this);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlBar);
    controlLayout->setContentsMargins(15, 10, 15, 10); // 增大边距
    controlLayout->setSpacing(15); // 增大控件间距

    // 4.1 播放/暂停按钮（带图标，Qt6 原生样式）
    m_btnPlayPause = new QPushButton(this);
    m_btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay)); // 初始播放图标
    m_btnPlayPause->setIconSize(QSize(24, 24));
    m_btnPlayPause->setFixedSize(36, 36);
    controlLayout->addWidget(m_btnPlayPause);

    // 4.2 音量控制
    m_btnVolume = new QPushButton(this);
    m_btnVolume->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    m_btnVolume->setIconSize(QSize(24, 24));
    m_btnVolume->setFixedSize(36, 36);
    controlLayout->addWidget(m_btnVolume);
    
    m_sliderVolume = new QSlider(Qt::Horizontal, this);
    m_sliderVolume->setRange(0, 100);
    m_sliderVolume->setValue(50); // 默认音量50%
    m_sliderVolume->setFixedWidth(100);
    controlLayout->addWidget(m_sliderVolume);

    // 4.3 进度条（保持Expanding策略，占满剩余空间）
    m_sliderProgress = new QSlider(Qt::Horizontal, this);
    m_sliderProgress->setRange(0, 0); // 初始范围为0
    m_sliderProgress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    controlLayout->addWidget(m_sliderProgress);

    // 4.4 时间显示（增大标签宽度）
    m_labelTime = new QLabel("0:00/0:00", this);
    m_labelTime->setFixedWidth(100); // 避免时间文字拥挤
    m_labelTime->setAlignment(Qt::AlignCenter);
    controlLayout->addWidget(m_labelTime);

    // 5. 复用构造函数的布局，添加视频控件和控制栏
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(m_contentWidget->layout());
    if (mainLayout) {
        mainLayout->addWidget(m_graphicsView); // 使用QGraphicsView显示视频
        mainLayout->addWidget(controlBar);    // 控制栏在下方
    }

    // 6. 关联信号槽（核心控制逻辑）
    // 播放/暂停按钮
    connect(m_btnPlayPause, &QPushButton::clicked, this, &FileViewSubWindow::onPlayPauseClicked);
    // 音量按钮
    connect(m_btnVolume, &QPushButton::clicked, this, [=]() {
        m_isMuted = !m_isMuted;
        m_audioOutput->setMuted(m_isMuted);
        m_btnVolume->setIcon(style()->standardIcon(m_isMuted ? QStyle::SP_MediaVolumeMuted : QStyle::SP_MediaVolume));
    });
    // 音量滑块
    connect(m_sliderVolume, &QSlider::valueChanged, this, [=](int value) {
        m_audioOutput->setVolume(value / 100.0);
    });
    // 进度条（拖动时标记，释放时更新位置，避免实时卡顿）
    connect(m_sliderProgress, &QSlider::sliderPressed, this, [=]() { m_isProgressDragging = true; });
    connect(m_sliderProgress, &QSlider::sliderReleased, this, [=]() {
        m_isProgressDragging = false;
        if (m_mediaPlayer) {
            m_mediaPlayer->setPosition(m_sliderProgress->value());
        }
    });
    
    // 媒体播放器信号
    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, [=](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::LoadedMedia) {
            // 获取视频信息
            m_videoDuration = m_mediaPlayer->duration();
            m_sliderProgress->setRange(0, m_videoDuration);
            
            // 获取视频分辨率
            const QSize resolution = m_mediaPlayer->metaData().value(QMediaMetaData::Resolution).toSize();
            m_videoWidth = resolution.width();
            m_videoHeight = resolution.height();
            
            qDebug() << "视频加载成功：" << filePath;
            qDebug() << "视频信息：宽=" << m_videoWidth << "高=" << m_videoHeight << "时长=" << m_videoDuration << "ms";
        }
    });
    
    // 视频帧变化信号（Qt 6使用QVideoSink）
    QVideoSink *videoSink = m_mediaPlayer->videoSink();
    if (videoSink) {
        connect(videoSink, &QVideoSink::videoFrameChanged, this, &FileViewSubWindow::onVideoFrameChanged);
    } else {
        qDebug() << "无法获取QVideoSink，视频帧处理将不可用";
    }
    
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, [=](qint64 position) {
        if (!m_sliderProgress || !m_labelTime || m_isProgressDragging) return;
        
        // 更新进度条
        m_sliderProgress->setValue(position);
        
        // 更新时间显示
        QString currentTime = formatTime(position);
        QString totalTime = formatTime(m_videoDuration);
        m_labelTime->setText(QString("%1/%2").arg(currentTime).arg(totalTime));
    });
    
    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged, this, [=](QMediaPlayer::PlaybackState state) {
        if (state == QMediaPlayer::PlayingState) {
            m_isPlaying = true;
            m_btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        } else {
            m_isPlaying = false;
            m_btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        }
    });
    
    connect(m_mediaPlayer, &QMediaPlayer::errorOccurred, this, [=](QMediaPlayer::Error error, const QString &errorString) {
        qDebug() << "视频播放错误：" << errorString;
        QMessageBox::warning(nullptr, "视频播放错误", 
            QString("无法播放视频文件：%1\n\n错误信息：%2").arg(filePath).arg(errorString));
    });

    // 7. 初始化视频帧处理定时器
    m_frameCaptureTimer = new QTimer(this);
    m_frameCaptureTimer->setInterval(33); // 约30fps
    connect(m_frameCaptureTimer, &QTimer::timeout, this, &FileViewSubWindow::onVideoFrameTimerTimeout);
    
    // 8. 设置初始状态
    m_isProcessingEnabled = true; // 初始启用视频帧处理，即使没有应用图像处理命令

    // 9. 自动播放
    m_mediaPlayer->play();
    m_isPlaying = true;
    m_btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));

    // 10. 设置视频模式标志
    m_isVideoMode = true;
    m_videoFilePath = filePath;

    qDebug() << "视频加载成功：" << filePath;
}

// ========== 视频控制槽函数实现 ==========
// 播放/暂停切换
void FileViewSubWindow::onPlayPauseClicked()
{
    if (!m_mediaPlayer || !m_isVideoMode) return;

    if (m_isPlaying) {
        m_mediaPlayer->pause();
        m_isPlaying = false;
        m_btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    } else {
        m_mediaPlayer->play();
        m_isPlaying = true;
        m_btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    }
}

// 进度条拖动（仅记录值，释放时更新）
void FileViewSubWindow::onProgressSliderChanged(int value)
{
    if (!m_mediaPlayer || !m_isVideoMode) return;
    // 拖动时不更新播放位置，仅在释放时更新
}

// 进度条释放（更新视频播放位置）
void FileViewSubWindow::onProgressSliderReleased()
{
    if (!m_mediaPlayer || !m_isVideoMode) return;

    m_isProgressDragging = false;
    qint64 position = m_sliderProgress->value();
    if (position < 0 || position > m_videoDuration) return;

    // 设置视频位置到指定时间点
    m_mediaPlayer->setPosition(position);
}
