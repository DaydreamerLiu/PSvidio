#ifndef FILEVIEWSUBWINDOW_H
#define FILEVIEWSUBWINDOW_H

#include <QMdiSubWindow>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QWheelEvent>
#include <QImage>
#include <QPushButton>
#include <QSlider>
#include <QHBoxLayout>
#include <QString>
#include <QList>
#include <QTimer>
#include <QMessageBox>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>
#include <QMediaMetaData>
#include <QVideoFrame>
#include <QVideoSink>
#include <QVideoFrameFormat>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include "imagecommand.h"

class FileViewSubWindow final : public QMdiSubWindow
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FileViewSubWindow)  // 禁用拷贝/移动，符合Qt6最佳实践

public:
    // 构造函数：explicit避免隐式转换，QWidget* parent = nullptr符合Qt6默认参数规范
    explicit FileViewSubWindow(const QString &filePath, QWidget *parent = nullptr);
    ~FileViewSubWindow() override = default;  // 显式默认析构，符合现代C++

    // 对外暴露缩放接口（供MainWindow的Slider调用）
    void setScaleFactor(int percent);  // 入参：1~500（对应1%~500%）
    int currentScalePercent() const;   // 获取当前缩放比例（%）

protected:
    // 重写滚轮事件：实现鼠标滚轮缩放
    void wheelEvent(QWheelEvent *event) override;

 signals:
    void scaleChanged(int percent);  // 缩放比例变化时触发，携带当前比例
    void commandApplied(ImageCommand *command);  // 命令应用或撤销/重做时触发


private slots:
    void onPlayPauseClicked();       // 播放/暂停切换
    void onProgressSliderChanged(int value); // 进度条拖动
    void onProgressSliderReleased(); // 进度条释放（避免实时卡顿）
    void onVideoFrameTimerTimeout(); // 视频帧定时处理槽函数
    void onVideoFrameChanged(const QVideoFrame &frame); // 视频帧变化槽函数（Qt 6）

public:
    // 获取当前图像（图片或视频帧）
    QImage getCurrentImage() const;
    // 获取原始图像
    QImage getOriginalImage() const;
    // 获取当前视频帧（仅视频模式）
    QImage getCurrentVideoFrame() const;

    // 图像处理相关方法
    void applyImageCommand(ImageCommand *command);
    bool canUndo() const;
    bool canRedo() const;
    void undo();
    void redo();
    ImageCommand* getCurrentCommand() const;  // 获取当前应用的命令
    
    // 检查是否为视频文件
    bool isVideoFile() const;
    // 应用图像处理命令到视频
    void applyImageCommandToVideo(ImageCommand *command);
    // 保存处理后的视频
    bool saveProcessedVideo(const QString &filePath);
    
    // 获取文件路径
    QString getFilePath() const;

private:
    // 加载媒体文件的私有方法
    void loadImage(const QString &filePath);  // 加载图片（JPG/PNG/BMP）
    void loadVideo(const QString &filePath);  // 加载视频（MP4/AVI/MOV）
    void updateImageDisplay();  // 刷新图片显示（核心：保持比例）
    void updateVideoDisplay();  // 刷新视频显示
    // 新增：格式化时间（毫秒转 分:秒，如 1:23）
    QString formatTime(qint64 ms);

    // 缩放相关成员变量
    QImage m_originalImage;     // 保存原始图片
    QImage m_currentImage;      // 当前显示的图片
    int m_scalePercent = 100;   // 当前缩放比例（默认100%）

    // 命令历史记录
    QList<ImageCommand*> m_commandHistory;
    int m_historyIndex = -1;     // 当前历史记录索引
    
    // 文件路径存储
    QString m_filePath;          // 存储文件路径

    // 成员变量：使用前向声明+初始化，遵循Qt6内存管理（父子机制）
    QWidget *m_contentWidget = nullptr;
    QLabel *m_imageLabel = nullptr;
    // 视频相关成员
    QPushButton *m_btnPlayPause = nullptr; // 播放/暂停按钮
    QPushButton *m_btnVolume = nullptr;    // 音量按钮
    QSlider *m_sliderProgress = nullptr;   // 进度条（0~视频时长）
    QSlider *m_sliderVolume = nullptr;     // 音量滑块
    QLabel *m_labelTime = nullptr;         // 时间显示（当前/总时长）
    bool m_isProgressDragging = false;     // 进度条拖动标记（避免卡顿）
    bool m_isMuted = false;                 // 静音状态
    
    // Qt多媒体相关成员变量
    bool m_isVideoMode = false;
    QString m_videoFilePath;
    QMediaPlayer *m_mediaPlayer = nullptr;  // 媒体播放器
    QVideoWidget *m_videoWidget = nullptr;  // 视频显示组件（用于捕获原始帧，隐藏）
    QAudioOutput *m_audioOutput = nullptr;  // 音频输出组件
    QTimer *m_frameCaptureTimer = nullptr;  // 视频帧捕获定时器
    
    // 视频信息
    int m_videoWidth = 0;                   // 视频宽度
    int m_videoHeight = 0;                  // 视频高度
    qint64 m_videoDuration = 0;             // 视频时长（毫秒）
    bool m_isPlaying = false;               // 播放状态
    
    // 视频帧缓存
    QImage m_currentVideoFrame;             // 当前处理后的视频帧
    QImage m_originalVideoFrame;            // 当前原始视频帧
    bool m_isProcessingEnabled = false;     // 是否启用视频处理
    
    // 视频显示相关组件
    QGraphicsScene *m_graphicsScene = nullptr;  // 图形场景
    QGraphicsView *m_graphicsView = nullptr;    // 图形视图
    QGraphicsPixmapItem *m_videoPixmapItem = nullptr;  // 视频帧显示项
};

#endif // FILEVIEWSUBWINDOW_H
