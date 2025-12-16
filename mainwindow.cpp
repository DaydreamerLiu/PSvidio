#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fileviewsubwindow.h" // 包含自定义子窗口头文件
#include "grayscalecommand.h"
#include "binarycommand.h"
#include "meanfiltercommand.h"
#include "gammacorrectioncommand.h"
#include "edgedetectioncommand.h"
#include "mosaiccommand.h"
#include <QFileDialog>
#include <QToolBar>
#include <QLabel>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_binaryThreshold(128)
    , m_gammaValue(1.0)
    , m_edgeThreshold(50)
{
    ui->setupUi(this);

    // 1. 设置标签页模式（核心：支持多文件标签切换）
    ui->mdiArea->setViewMode(QMdiArea::TabbedView);


    // 3. 标签栏优化（Qt6 交互体验增强）
    ui->mdiArea->setTabsClosable(true);    // 标签带关闭按钮
    ui->mdiArea->setTabsMovable(true);     // 标签可拖动排序
    ui->mdiArea->setTabPosition(QTabWidget::North);  // 标签在顶部（Qt6 正确枚举）
    ui->mdiArea->setTabShape(QTabWidget::Rounded);   // 圆角标签（Qt6 美化）

    // 找到MDI区域内的标签栏（QTabWidget）
    QTabWidget *mdiTabBar = ui->mdiArea->findChild<QTabWidget*>();
    if (mdiTabBar) {
        // 关联QTabWidget的关闭信号 → 关闭对应MDI子窗口
        connect(mdiTabBar, &QTabWidget::tabCloseRequested, this, [=](int index) {
            // 确保索引有效
            if (index >= 0 && index < ui->mdiArea->subWindowList().size()) {
                QMdiSubWindow *subWin = ui->mdiArea->subWindowList().at(index);
                if (subWin) {
                    subWin->close();       // 关闭子窗口（自动从MDI移除）
                    subWin->deleteLater(); // 安全销毁，避免内存泄漏
                }
            }
        });
    }


    // 4. MDI窗口行为优化
    ui->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    // 打印Qt版本（调试用，Qt6.9.2 宏QT_VERSION_STR）
    qDebug() << "Qt 版本：" << QT_VERSION_STR;

    // 创建定时器用于滑块延迟处理
    m_timer = new QTimer(this);
    m_timer->setInterval(300); // 设置300ms延迟
    m_timer->setSingleShot(true); // 单次触发
    
    // 连接信号和槽
    connect(ui->mdiArea, &QMdiArea::subWindowActivated, this, [=](QMdiSubWindow *subWindow) {
        if (subWindow) {
            FileViewSubWindow *imageWin = qobject_cast<FileViewSubWindow*>(subWindow->widget());
            if (imageWin) {
                // 连接命令应用信号
                connect(imageWin, &FileViewSubWindow::commandApplied, this, &MainWindow::onCommandApplied);
                // 初始化当前命令
                onCommandApplied(imageWin->getCurrentCommand());
            }
        }
    });
    
    // 5. 添加工具栏滑块控件
    // 创建一个新的工具栏用于放置滑块控件
    QToolBar *toolBar = addToolBar("ImageToolBar");
    toolBar->setVisible(true); // 确保工具栏默认可见
    toolBar->setFloatable(true); // 允许工具栏浮动
    toolBar->setMovable(true); // 允许工具栏移动
    
    // 创建二值化阈值控件
    binaryLabel = new QLabel("二值化阈值：", this);
    m_binaryThresholdSlider = new QSlider(Qt::Horizontal, this);
    m_binaryThresholdSlider->setRange(0, 255);
    m_binaryThresholdSlider->setValue(m_binaryThreshold);
    m_binaryThresholdSlider->setToolTip("二值化阈值 (0-255)");
    m_binaryThresholdSlider->setMinimumWidth(150); // 设置最小宽度
    m_binaryThresholdSlider->setMaximumWidth(200); // 设置最大宽度
    m_binaryThresholdSlider->setFixedHeight(20); // 设置固定高度
    m_binaryThresholdSlider->setEnabled(false); // 默认不可用
    binaryValueLabel = new QLabel(QString::number(m_binaryThreshold), this);
    binaryValueLabel->setFixedWidth(40); // 固定宽度以对齐
    binaryValueLabel->setAlignment(Qt::AlignCenter);
    connect(m_binaryThresholdSlider, &QSlider::valueChanged, this, &MainWindow::on_binaryThresholdSlider_valueChanged);
    connect(m_binaryThresholdSlider, &QSlider::sliderPressed, this, &MainWindow::on_sliderPressed);
    connect(m_binaryThresholdSlider, &QSlider::sliderReleased, this, &MainWindow::on_binaryThresholdSlider_released);
    
    // 创建伽马变换值控件
    gammaLabel = new QLabel("伽马值：", this);
    m_gammaValueSlider = new QSlider(Qt::Horizontal, this);
    m_gammaValueSlider->setRange(1, 30); // 1-30表示1.0-3.0
    m_gammaValueSlider->setValue(m_gammaValue * 10);
    m_gammaValueSlider->setToolTip("伽马值 (1.0-3.0)");
    m_gammaValueSlider->setMinimumWidth(150); // 设置最小宽度
    m_gammaValueSlider->setMaximumWidth(200); // 设置最大宽度
    m_gammaValueSlider->setFixedHeight(20); // 设置固定高度
    m_gammaValueSlider->setEnabled(false); // 默认不可用
    gammaValueLabel = new QLabel(QString::number(m_gammaValue, 'f', 1), this);
    gammaValueLabel->setFixedWidth(40); // 固定宽度以对齐
    gammaValueLabel->setAlignment(Qt::AlignCenter);
    connect(m_gammaValueSlider, &QSlider::valueChanged, this, &MainWindow::on_gammaValueSlider_valueChanged);
    connect(m_gammaValueSlider, &QSlider::sliderPressed, this, &MainWindow::on_sliderPressed);
    connect(m_gammaValueSlider, &QSlider::sliderReleased, this, &MainWindow::on_gammaValueSlider_released);
    
    // 创建边缘检测阈值控件
    edgeLabel = new QLabel("边缘阈值：", this);
    m_edgeThresholdSlider = new QSlider(Qt::Horizontal, this);
    m_edgeThresholdSlider->setRange(0, 200); // 0-200的阈值范围
    m_edgeThresholdSlider->setValue(m_edgeThreshold);
    m_edgeThresholdSlider->setToolTip("边缘检测阈值 (0-200)");
    m_edgeThresholdSlider->setMinimumWidth(150); // 设置最小宽度
    m_edgeThresholdSlider->setMaximumWidth(200); // 设置最大宽度
    m_edgeThresholdSlider->setFixedHeight(20); // 设置固定高度
    m_edgeThresholdSlider->setEnabled(false); // 默认不可用
    edgeValueLabel = new QLabel(QString::number(m_edgeThreshold), this);
    edgeValueLabel->setFixedWidth(40); // 固定宽度以对齐
    edgeValueLabel->setAlignment(Qt::AlignCenter);
    connect(m_edgeThresholdSlider, &QSlider::valueChanged, this, &MainWindow::on_edgeThresholdSlider_valueChanged);
    connect(m_edgeThresholdSlider, &QSlider::sliderPressed, this, &MainWindow::on_sliderPressed);
    connect(m_edgeThresholdSlider, &QSlider::sliderReleased, this, &MainWindow::on_edgeThresholdSlider_released);
    
    // 将控件添加到工具栏
    toolBar->addWidget(binaryLabel);
    toolBar->addWidget(m_binaryThresholdSlider);
    toolBar->addWidget(binaryValueLabel);
    toolBar->addWidget(gammaLabel);
    toolBar->addWidget(m_gammaValueSlider);
    toolBar->addWidget(gammaValueLabel);
    toolBar->addWidget(edgeLabel);
    toolBar->addWidget(m_edgeThresholdSlider);
    toolBar->addWidget(edgeValueLabel);
    
    // 默认隐藏所有标签、滑块和数值显示
    binaryLabel->setVisible(false);
    m_binaryThresholdSlider->setVisible(false);
    binaryValueLabel->setVisible(false);
    gammaLabel->setVisible(false);
    m_gammaValueSlider->setVisible(false);
    gammaValueLabel->setVisible(false);
    edgeLabel->setVisible(false);
    m_edgeThresholdSlider->setVisible(false);
    edgeValueLabel->setVisible(false);
    
    // 确保工具栏在所有其他控件之上
    toolBar->raise();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 获取当前激活的子窗口（图片或视频）
FileViewSubWindow* MainWindow::currentImageSubWindow()
{
    QMdiSubWindow *activeWin = ui->mdiArea->activeSubWindow();
    if (!activeWin) return nullptr;
    // 直接转换为自定义子窗口类型，而不是转换widget
    return qobject_cast<FileViewSubWindow*>(activeWin);
}

void MainWindow::on_actionNew_new_triggered()
{

}


void MainWindow::on_actionOpen_O_triggered()
{
    // Qt6.9.2 文件对话框（现代风格，支持原生系统对话框）
    const QStringList filePaths = QFileDialog::getOpenFileNames(
        this,
        tr("选择媒体文件"),          // 对话框标题（国际化）
        QDir::homePath(),           // 默认路径（用户主目录）
        tr("媒体文件 (*.jpg *.jpeg *.png *.bmp *.mp4 *.avi *.mov);;所有文件 (*.*)")  // 过滤规则
        );

    // 无文件选择时直接返回
    if (filePaths.isEmpty()) {
        return;
    }

    // 为每个文件创建MDI子窗口
    for (const QString &filePath : filePaths) {
        FileViewSubWindow *subWindow = new FileViewSubWindow(filePath, this);
        subWindow->setAttribute(Qt::WA_DeleteOnClose);
        ui->mdiArea->addSubWindow(subWindow);
        subWindow->showMaximized();  // 默认为最大化状态
    }

    // 激活第一个打开的窗口（Qt6 QMdiArea 子窗口列表）
    if (!ui->mdiArea->subWindowList().isEmpty()) {
        ui->mdiArea->setActiveSubWindow(ui->mdiArea->subWindowList().first());
    }
}

void MainWindow::on_horizontalSliderScale_valueChanged(int value)
{
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;

    // 更新图片缩放
    imageWin->setScaleFactor(value);
    // 实时显示当前缩放比例
    ui->labelScale->setText(tr("当前缩放：%1%").arg(value));
}


void MainWindow::on_mdiArea_subWindowActivated(QMdiSubWindow *arg1)
{
    Q_UNUSED(arg1);
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) {
        // 无图片窗口时禁用Slider
        ui->horizontalSliderScale->setEnabled(false);
        ui->labelScale->setText(tr("当前缩放：--"));
        return;
    }

    // 启用Slider并同步当前缩放比例
    ui->horizontalSliderScale->setEnabled(true);
    int currentPercent = imageWin->currentScalePercent();
    ui->horizontalSliderScale->setValue(currentPercent);
    ui->labelScale->setText(tr("当前缩放：%1%").arg(currentPercent));
}

// 灰度化
void MainWindow::on_action_G_triggered()
{
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;

    // 隐藏所有滑块、标签和数值显示
    binaryLabel->setVisible(false);
    m_binaryThresholdSlider->setVisible(false);
    m_binaryThresholdSlider->setEnabled(false);
    binaryValueLabel->setVisible(false);
    gammaLabel->setVisible(false);
    m_gammaValueSlider->setVisible(false);
    m_gammaValueSlider->setEnabled(false);
    gammaValueLabel->setVisible(false);
    edgeLabel->setVisible(false);
    m_edgeThresholdSlider->setVisible(false);
    m_edgeThresholdSlider->setEnabled(false);
    edgeValueLabel->setVisible(false);

    // 根据窗口类型选择命令应用方法
    GrayscaleCommand *command = new GrayscaleCommand(imageWin->getCurrentImage());
    if (imageWin->isVideoFile()) {
        imageWin->applyImageCommandToVideoFrame(command);
    } else {
        imageWin->applyImageCommand(command);
    }
}

// 二值化
void MainWindow::on_action_T_triggered()
{
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;

    // 显示二值化阈值滑块、标签和数值显示，隐藏其他控件
    binaryLabel->setVisible(true);
    m_binaryThresholdSlider->setVisible(true);
    m_binaryThresholdSlider->setEnabled(true);
    binaryValueLabel->setVisible(true);
    gammaLabel->setVisible(false);
    m_gammaValueSlider->setVisible(false);
    m_gammaValueSlider->setEnabled(false);
    gammaValueLabel->setVisible(false);
    edgeLabel->setVisible(false);
    m_edgeThresholdSlider->setVisible(false);
    m_edgeThresholdSlider->setEnabled(false);
    edgeValueLabel->setVisible(false);

    // 更新数值显示
    binaryValueLabel->setText(QString::number(m_binaryThreshold));
    
    // 根据窗口类型选择命令应用方法
    BinaryCommand *command = new BinaryCommand(imageWin->getCurrentImage(), m_binaryThreshold);
    if (imageWin->isVideoFile()) {
        imageWin->applyImageCommandToVideoFrame(command);
    } else {
        imageWin->applyImageCommand(command);
    }
}

// 3×3均值滤波
void MainWindow::on_action_2_triggered()
{
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;

    // 隐藏所有滑块、标签和数值显示
    binaryLabel->setVisible(false);
    m_binaryThresholdSlider->setVisible(false);
    m_binaryThresholdSlider->setEnabled(false);
    binaryValueLabel->setVisible(false);
    gammaLabel->setVisible(false);
    m_gammaValueSlider->setVisible(false);
    m_gammaValueSlider->setEnabled(false);
    gammaValueLabel->setVisible(false);
    edgeLabel->setVisible(false);
    m_edgeThresholdSlider->setVisible(false);
    m_edgeThresholdSlider->setEnabled(false);
    edgeValueLabel->setVisible(false);

    // 根据窗口类型选择命令应用方法
    MeanFilterCommand *command = new MeanFilterCommand(imageWin->getCurrentImage());
    if (imageWin->isVideoFile()) {
        imageWin->applyImageCommandToVideoFrame(command);
    } else {
        imageWin->applyImageCommand(command);
    }
}

// 伽马变换
void MainWindow::on_action_3_triggered()
{
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;

    // 显示伽马值滑块、标签和数值显示，隐藏其他控件
    binaryLabel->setVisible(false);
    m_binaryThresholdSlider->setVisible(false);
    m_binaryThresholdSlider->setEnabled(false);
    binaryValueLabel->setVisible(false);
    gammaLabel->setVisible(true);
    m_gammaValueSlider->setVisible(true);
    m_gammaValueSlider->setEnabled(true);
    gammaValueLabel->setVisible(true);
    edgeLabel->setVisible(false);
    m_edgeThresholdSlider->setVisible(false);
    m_edgeThresholdSlider->setEnabled(false);
    edgeValueLabel->setVisible(false);

    // 更新数值显示
    gammaValueLabel->setText(QString::number(m_gammaValue, 'f', 1));
    
    // 根据窗口类型选择命令应用方法
    GammaCorrectionCommand *command = new GammaCorrectionCommand(imageWin->getCurrentImage(), m_gammaValue);
    if (imageWin->isVideoFile()) {
        imageWin->applyImageCommandToVideoFrame(command);
    } else {
        imageWin->applyImageCommand(command);
    }
}

// 边缘检测
void MainWindow::on_action_4_triggered()
{
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;

    // 显示边缘检测阈值滑块、标签和数值显示，隐藏其他控件
    binaryLabel->setVisible(false);
    m_binaryThresholdSlider->setVisible(false);
    m_binaryThresholdSlider->setEnabled(false);
    binaryValueLabel->setVisible(false);
    gammaLabel->setVisible(false);
    m_gammaValueSlider->setVisible(false);
    m_gammaValueSlider->setEnabled(false);
    gammaValueLabel->setVisible(false);
    edgeLabel->setVisible(true);
    m_edgeThresholdSlider->setVisible(true);
    m_edgeThresholdSlider->setEnabled(true);
    edgeValueLabel->setVisible(true);

    // 更新数值显示
    edgeValueLabel->setText(QString::number(m_edgeThreshold));
    
    // 根据窗口类型选择命令应用方法
    EdgeDetectionCommand *command = new EdgeDetectionCommand(imageWin->getCurrentImage(), m_edgeThreshold);
    if (imageWin->isVideoFile()) {
        imageWin->applyImageCommandToVideoFrame(command);
    } else {
        imageWin->applyImageCommand(command);
    }
}

// 局部马赛克
void MainWindow::on_action_Mosaic_triggered()
{
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;

    // 隐藏所有滑块、标签和数值显示
    binaryLabel->setVisible(false);
    m_binaryThresholdSlider->setVisible(false);
    m_binaryThresholdSlider->setEnabled(false);
    binaryValueLabel->setVisible(false);
    gammaLabel->setVisible(false);
    m_gammaValueSlider->setVisible(false);
    m_gammaValueSlider->setEnabled(false);
    gammaValueLabel->setVisible(false);
    edgeLabel->setVisible(false);
    m_edgeThresholdSlider->setVisible(false);
    m_edgeThresholdSlider->setEnabled(false);
    edgeValueLabel->setVisible(false);

    // 创建并应用局部马赛克命令
    // 这里使用固定区域和块大小，实际应用中可以让用户选择区域
    QRect region(50, 50, 200, 200); // 示例区域：(x,y,width,height)
    int blockSize = 10; // 马赛克块大小
    MosaicCommand *command = new MosaicCommand(imageWin->getCurrentImage(), region, blockSize);
    
    // 根据窗口类型选择命令应用方法
    if (imageWin->isVideoFile()) {
        imageWin->applyImageCommandToVideoFrame(command);
    } else {
        imageWin->applyImageCommand(command);
    }
}

// 撤销
void MainWindow::on_action_Z_triggered()
{
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;

    imageWin->undo();
    
    // 更新滑块状态
    onCommandApplied(imageWin->getCurrentCommand());
}

// 重做
void MainWindow::on_action_Y_triggered()
{
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;

    imageWin->redo();
    
    // 更新滑块状态
    onCommandApplied(imageWin->getCurrentCommand());
}

// 滑块按下时的处理
void MainWindow::on_sliderPressed()
{
    // 停止定时器，避免在拖动过程中处理
    m_timer->stop();
}

// 二值化阈值滑块变化（仅更新显示）
void MainWindow::on_binaryThresholdSlider_valueChanged(int value)
{
    m_binaryThreshold = value;
    // 只更新数值显示，不立即处理图像
    binaryValueLabel->setText(QString::number(value));
}

// 二值化阈值滑块释放时的处理
void MainWindow::on_binaryThresholdSlider_released()
{
    // 启动定时器，延迟处理图像
    m_timer->start(300);
    // 连接定时器超时信号到处理函数
    connect(m_timer, &QTimer::timeout, this, [=]() {
        FileViewSubWindow *imageWin = currentImageSubWindow();
        if (!imageWin) return;
        // 重新应用二值化命令，使用原始图像而非当前图像
        BinaryCommand *command = new BinaryCommand(imageWin->getOriginalImage(), m_binaryThreshold);
        if (imageWin->isVideoFile()) {
            imageWin->applyImageCommandToVideoFrame(command);
        } else {
            imageWin->applyImageCommand(command);
        }
        // 断开连接，避免重复处理
        disconnect(m_timer, &QTimer::timeout, nullptr, nullptr);
    });
}

// 伽马值滑块变化（仅更新显示）
void MainWindow::on_gammaValueSlider_valueChanged(int value)
{
    m_gammaValue = value / 10.0;
    // 只更新数值显示，不立即处理图像
    gammaValueLabel->setText(QString::number(m_gammaValue, 'f', 1));
}

// 伽马值滑块释放时的处理
void MainWindow::on_gammaValueSlider_released()
{
    // 启动定时器，延迟处理图像
    m_timer->start(300);
    // 连接定时器超时信号到处理函数
    connect(m_timer, &QTimer::timeout, this, [=]() {
        FileViewSubWindow *imageWin = currentImageSubWindow();
        if (!imageWin) return;
        // 重新应用伽马变换命令，使用原始图像而非当前图像
        GammaCorrectionCommand *command = new GammaCorrectionCommand(imageWin->getOriginalImage(), m_gammaValue);
        if (imageWin->isVideoFile()) {
            imageWin->applyImageCommandToVideoFrame(command);
        } else {
            imageWin->applyImageCommand(command);
        }
        // 断开连接，避免重复处理
        disconnect(m_timer, &QTimer::timeout, nullptr, nullptr);
    });
}

// 边缘检测阈值滑块变化（仅更新显示）
void MainWindow::on_edgeThresholdSlider_valueChanged(int value)
{
    m_edgeThreshold = value;
    // 只更新数值显示，不立即处理图像
    edgeValueLabel->setText(QString::number(value));
}

// 边缘检测阈值滑块释放时的处理
void MainWindow::on_edgeThresholdSlider_released()
{
    // 启动定时器，延迟处理图像
    m_timer->start(300);
    // 连接定时器超时信号到处理函数
    connect(m_timer, &QTimer::timeout, this, [=]() {
        FileViewSubWindow *imageWin = currentImageSubWindow();
        if (!imageWin) return;
        // 重新应用边缘检测命令，使用原始图像而非当前图像
        EdgeDetectionCommand *command = new EdgeDetectionCommand(imageWin->getOriginalImage(), m_edgeThreshold);
        if (imageWin->isVideoFile()) {
            imageWin->applyImageCommandToVideoFrame(command);
        } else {
            imageWin->applyImageCommand(command);
        }
        // 断开连接，避免重复处理
        disconnect(m_timer, &QTimer::timeout, nullptr, nullptr);
    });
}

// 处理命令应用信号
void MainWindow::onCommandApplied(ImageCommand *command)
{
    // 默认隐藏所有滑块、标签和数值显示
    binaryLabel->setVisible(false);
    m_binaryThresholdSlider->setVisible(false);
    m_binaryThresholdSlider->setEnabled(false);
    binaryValueLabel->setVisible(false);
    gammaLabel->setVisible(false);
    m_gammaValueSlider->setVisible(false);
    m_gammaValueSlider->setEnabled(false);
    gammaValueLabel->setVisible(false);
    edgeLabel->setVisible(false);
    m_edgeThresholdSlider->setVisible(false);
    m_edgeThresholdSlider->setEnabled(false);
    edgeValueLabel->setVisible(false);
    
    // 根据命令类型更新滑块、标签和数值显示
    if (BinaryCommand *binaryCommand = dynamic_cast<BinaryCommand*>(command)) {
        // 二值化命令
        m_binaryThreshold = binaryCommand->threshold();
        m_binaryThresholdSlider->setValue(m_binaryThreshold);
        binaryValueLabel->setText(QString::number(m_binaryThreshold));
        
        // 显示二值化控件
        binaryLabel->setVisible(true);
        m_binaryThresholdSlider->setVisible(true);
        m_binaryThresholdSlider->setEnabled(true);
        binaryValueLabel->setVisible(true);
    } else if (GammaCorrectionCommand *gammaCommand = dynamic_cast<GammaCorrectionCommand*>(command)) {
        // 伽马变换命令
        m_gammaValue = gammaCommand->gamma();
        m_gammaValueSlider->setValue(m_gammaValue * 10);
        gammaValueLabel->setText(QString::number(m_gammaValue, 'f', 1));
        
        // 显示伽马变换控件
        gammaLabel->setVisible(true);
        m_gammaValueSlider->setVisible(true);
        m_gammaValueSlider->setEnabled(true);
        gammaValueLabel->setVisible(true);
    } else if (EdgeDetectionCommand *edgeCommand = dynamic_cast<EdgeDetectionCommand*>(command)) {
        // 边缘检测命令
        m_edgeThreshold = edgeCommand->threshold();
        m_edgeThresholdSlider->setValue(m_edgeThreshold);
        edgeValueLabel->setText(QString::number(m_edgeThreshold));
        
        // 显示边缘检测控件
        edgeLabel->setVisible(true);
        m_edgeThresholdSlider->setVisible(true);
        m_edgeThresholdSlider->setEnabled(true);
        edgeValueLabel->setVisible(true);
    }
}

