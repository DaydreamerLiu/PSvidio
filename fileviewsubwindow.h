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
#include <QUrl>
#include <QWheelEvent>
#include <QImage>
#include <QPushButton>
#include <QSlider>
#include <QHBoxLayout>
#include <QString>
#include <QList>
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
    // 新增视频控制槽函数
    void onPlayPauseClicked();       // 播放/暂停切换
    void onVolumeSliderChanged(int value); // 音量调节
    void onProgressSliderChanged(int value); // 进度条拖动
    void onProgressSliderReleased(); // 进度条释放（避免实时卡顿）
    void onPlayerStateChanged(QMediaPlayer::PlaybackState state); // 播放状态变化
    void onDurationChanged(qint64 duration); // 视频时长变化
    void onPositionChanged(qint64 position); // 播放位置变化

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
    // 应用图像处理命令到当前视频帧
    void applyImageCommandToVideoFrame(ImageCommand *command);

private:
    // 加载媒体文件的私有方法
    void loadImage(const QString &filePath);  // 加载图片（JPG/PNG/BMP）
    void loadVideo(const QString &filePath);  // 加载视频（MP4/AVI/MOV）
    void updateImageDisplay();  // 刷新图片显示（核心：保持比例）
    // 新增：格式化时间（毫秒转 分:秒，如 1:23）
    QString formatTime(qint64 ms);

    // 缩放相关成员变量
    QImage m_originalImage;     // 保存原始图片
    QImage m_currentImage;      // 当前显示的图片
    int m_scalePercent = 100;   // 当前缩放比例（默认100%）

    // 命令历史记录
    QList<ImageCommand*> m_commandHistory;
    int m_historyIndex = -1;     // 当前历史记录索引

    // 成员变量：使用前向声明+初始化，遵循Qt6内存管理（父子机制）
    QWidget *m_contentWidget = nullptr;
    QLabel *m_imageLabel = nullptr;
    // 视频相关成员
    QMediaPlayer *m_mediaPlayer = nullptr;
    QVideoWidget *m_videoWidget = nullptr;
    QAudioOutput *m_audioOutput = nullptr;   // Qt6 音频输出（替代原setVolume）
    QPushButton *m_btnPlayPause = nullptr; // 播放/暂停按钮
    QSlider *m_sliderVolume = nullptr;     // 音量滑块（0~100）
    QSlider *m_sliderProgress = nullptr;   // 进度条（0~视频时长）
    QLabel *m_labelTime = nullptr;         // 时间显示（当前/总时长）
    bool m_isProgressDragging = false;     // 进度条拖动标记（避免卡顿）
};

#endif // FILEVIEWSUBWINDOW_H
