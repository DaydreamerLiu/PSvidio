#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fileviewsubwindow.h" // 包含自定义子窗口头文件
#include "grayscalecommand.h"
#include "binarycommand.h"
#include "meanfiltercommand.h"
#include "gammacorrectioncommand.h"
#include "edgedetectioncommand.h"
#include "mosaiccommand.h"
#include "collapsiblewidget.h" // 可折叠控件
#include <QFileDialog>
#include <QToolBar>
#include <QLabel>
#include <QTimer>
#include <QStyle>
#include <QFile>
#include <QTextStream>
#include <QGroupBox>

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
        
        // 添加标签页切换时的文件信息更新
        connect(mdiTabBar, &QTabWidget::currentChanged, this, [=](int index) {
            if (index >= 0 && index < ui->mdiArea->subWindowList().size()) {
                QMdiSubWindow *subWin = ui->mdiArea->subWindowList().at(index);
                if (subWin) {
                    FileViewSubWindow *imageWin = qobject_cast<FileViewSubWindow*>(subWin);
                    if (imageWin) {
                        updateFileInfo(imageWin);
                    }
                }
            } else {
                updateFileInfo(nullptr);
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
            FileViewSubWindow *imageWin = qobject_cast<FileViewSubWindow*>(subWindow);
            if (imageWin) {
                // 连接命令应用信号
                connect(imageWin, &FileViewSubWindow::commandApplied, this, &MainWindow::onCommandApplied, Qt::UniqueConnection);
                // 初始化当前命令
                onCommandApplied(imageWin->getCurrentCommand());
                // 更新文件信息
                updateFileInfo(imageWin);
            }
        } else {
            // 没有激活窗口时清空文件信息
            updateFileInfo(nullptr);
        }
    });
    
    // 5. 应用现代样式
    applyModernStyle();
    
    // 6. 创建主工具栏（包含所有操作的快捷按钮）
    setupMainToolBar();
    
    // 7. 创建可折叠的参数调节工具栏
    setupParameterToolBar();
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
        // 更新文件信息
        FileViewSubWindow *firstWin = qobject_cast<FileViewSubWindow*>(ui->mdiArea->subWindowList().first());
        if (firstWin) {
            updateFileInfo(firstWin);
        }
    }
}

// 保存处理后的视频
void MainWindow::on_actionSaveVideo_triggered()
{
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin || !imageWin->isVideoFile()) {
        return;
    }
    
    // 打开文件对话框让用户选择保存位置
    QString dirPath = QFileDialog::getExistingDirectory(
        this,
        tr("选择保存目录"),
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    
    if (dirPath.isEmpty()) {
        return;
    }
    
    // 保存处理后的视频
    bool success = imageWin->saveProcessedVideo(dirPath);
    
    // 显示保存结果
    if (success) {
        qDebug() << "视频保存成功：" << dirPath;
    } else {
        qDebug() << "视频保存失败";
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

    // 折叠所有参数控件
    CollapsibleWidget *binaryCollapsible = findChild<CollapsibleWidget*>("binaryCollapsible");
    CollapsibleWidget *gammaCollapsible = findChild<CollapsibleWidget*>("gammaCollapsible");
    CollapsibleWidget *edgeCollapsible = findChild<CollapsibleWidget*>("edgeCollapsible");
    
    if (binaryCollapsible) binaryCollapsible->setCollapsed(true);
    if (gammaCollapsible) gammaCollapsible->setCollapsed(true);
    if (edgeCollapsible) edgeCollapsible->setCollapsed(true);

    // 根据窗口类型选择命令应用方法
    GrayscaleCommand *command = new GrayscaleCommand(imageWin->getCurrentImage());
    if (imageWin->isVideoFile()) {
        imageWin->applyImageCommandToVideo(command);
    } else {
        imageWin->applyImageCommand(command);
    }
}

// 二值化
void MainWindow::on_action_T_triggered()
{
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;

    // 展开二值化参数控件
    CollapsibleWidget *binaryCollapsible = findChild<CollapsibleWidget*>("binaryCollapsible");
    if (binaryCollapsible) binaryCollapsible->setCollapsed(false);
    
    // 折叠其他参数控件
    CollapsibleWidget *gammaCollapsible = findChild<CollapsibleWidget*>("gammaCollapsible");
    CollapsibleWidget *edgeCollapsible = findChild<CollapsibleWidget*>("edgeCollapsible");
    if (gammaCollapsible) gammaCollapsible->setCollapsed(true);
    if (edgeCollapsible) edgeCollapsible->setCollapsed(true);
    
    // 更新数值显示
    binaryValueLabel->setText(QString::number(m_binaryThreshold));
    
    // 根据窗口类型选择命令应用方法
    BinaryCommand *command = new BinaryCommand(imageWin->getCurrentImage(), m_binaryThreshold);
    if (imageWin->isVideoFile()) {
        imageWin->applyImageCommandToVideo(command);
    } else {
        imageWin->applyImageCommand(command);
    }
}

// 3×3均值滤波
void MainWindow::on_action_2_triggered()
{
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;

    // 折叠所有参数控件
    CollapsibleWidget *binaryCollapsible = findChild<CollapsibleWidget*>("binaryCollapsible");
    CollapsibleWidget *gammaCollapsible = findChild<CollapsibleWidget*>("gammaCollapsible");
    CollapsibleWidget *edgeCollapsible = findChild<CollapsibleWidget*>("edgeCollapsible");
    
    if (binaryCollapsible) binaryCollapsible->setCollapsed(true);
    if (gammaCollapsible) gammaCollapsible->setCollapsed(true);
    if (edgeCollapsible) edgeCollapsible->setCollapsed(true);

    // 根据窗口类型选择命令应用方法
    MeanFilterCommand *command = new MeanFilterCommand(imageWin->getCurrentImage());
    if (imageWin->isVideoFile()) {
        imageWin->applyImageCommandToVideo(command);
    } else {
        imageWin->applyImageCommand(command);
    }
}

// 伽马变换
void MainWindow::on_action_3_triggered()
{
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;

    // 展开伽马变换参数控件
    CollapsibleWidget *gammaCollapsible = findChild<CollapsibleWidget*>("gammaCollapsible");
    if (gammaCollapsible) gammaCollapsible->setCollapsed(false);
    
    // 折叠其他参数控件
    CollapsibleWidget *binaryCollapsible = findChild<CollapsibleWidget*>("binaryCollapsible");
    CollapsibleWidget *edgeCollapsible = findChild<CollapsibleWidget*>("edgeCollapsible");
    if (binaryCollapsible) binaryCollapsible->setCollapsed(true);
    if (edgeCollapsible) edgeCollapsible->setCollapsed(true);

    // 更新数值显示
    gammaValueLabel->setText(QString::number(m_gammaValue, 'f', 1));
    
    // 根据窗口类型选择命令应用方法
    GammaCorrectionCommand *command = new GammaCorrectionCommand(imageWin->getCurrentImage(), m_gammaValue);
    if (imageWin->isVideoFile()) {
        imageWin->applyImageCommandToVideo(command);
    } else {
        imageWin->applyImageCommand(command);
    }
}

// 边缘检测
void MainWindow::on_action_4_triggered()
{
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;

    // 展开边缘检测参数控件
    CollapsibleWidget *edgeCollapsible = findChild<CollapsibleWidget*>("edgeCollapsible");
    if (edgeCollapsible) edgeCollapsible->setCollapsed(false);
    
    // 折叠其他参数控件
    CollapsibleWidget *binaryCollapsible = findChild<CollapsibleWidget*>("binaryCollapsible");
    CollapsibleWidget *gammaCollapsible = findChild<CollapsibleWidget*>("gammaCollapsible");
    if (binaryCollapsible) binaryCollapsible->setCollapsed(true);
    if (gammaCollapsible) gammaCollapsible->setCollapsed(true);

    // 更新数值显示
    edgeValueLabel->setText(QString::number(m_edgeThreshold));
    
    // 根据窗口类型选择命令应用方法
    EdgeDetectionCommand *command = new EdgeDetectionCommand(imageWin->getCurrentImage(), m_edgeThreshold);
    if (imageWin->isVideoFile()) {
        imageWin->applyImageCommandToVideo(command);
    } else {
        imageWin->applyImageCommand(command);
    }
}

// 局部马赛克
void MainWindow::on_action_Mosaic_triggered()
{
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;

    // 折叠所有参数控件
    CollapsibleWidget *binaryCollapsible = findChild<CollapsibleWidget*>("binaryCollapsible");
    CollapsibleWidget *gammaCollapsible = findChild<CollapsibleWidget*>("gammaCollapsible");
    CollapsibleWidget *edgeCollapsible = findChild<CollapsibleWidget*>("edgeCollapsible");
    
    if (binaryCollapsible) binaryCollapsible->setCollapsed(true);
    if (gammaCollapsible) gammaCollapsible->setCollapsed(true);
    if (edgeCollapsible) edgeCollapsible->setCollapsed(true);

    // 创建并应用局部马赛克命令
    // 这里使用固定区域和块大小，实际应用中可以让用户选择区域
    QRect region(50, 50, 200, 200); // 示例区域：(x,y,width,height)
    int blockSize = 10; // 马赛克块大小
    MosaicCommand *command = new MosaicCommand(imageWin->getCurrentImage(), region, blockSize);
    
    // 根据窗口类型选择命令应用方法
    if (imageWin->isVideoFile()) {
        imageWin->applyImageCommandToVideo(command);
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
            imageWin->applyImageCommandToVideo(command);
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
            imageWin->applyImageCommandToVideo(command);
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
            imageWin->applyImageCommandToVideo(command);
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
    // 首先折叠所有参数控件
    CollapsibleWidget *binaryCollapsible = findChild<CollapsibleWidget*>("binaryCollapsible");
    CollapsibleWidget *gammaCollapsible = findChild<CollapsibleWidget*>("gammaCollapsible");
    CollapsibleWidget *edgeCollapsible = findChild<CollapsibleWidget*>("edgeCollapsible");
    
    if (binaryCollapsible) binaryCollapsible->setCollapsed(true);
    if (gammaCollapsible) gammaCollapsible->setCollapsed(true);
    if (edgeCollapsible) edgeCollapsible->setCollapsed(true);
    
    // 根据命令类型更新滑块值并展开对应的折叠控件
    if (BinaryCommand *binaryCommand = dynamic_cast<BinaryCommand*>(command)) {
        // 二值化命令
        m_binaryThreshold = binaryCommand->threshold();
        m_binaryThresholdSlider->setValue(m_binaryThreshold);
        binaryValueLabel->setText(QString::number(m_binaryThreshold));
        
        // 展开二值化参数控件
        if (binaryCollapsible) binaryCollapsible->setCollapsed(false);
        
    } else if (GammaCorrectionCommand *gammaCommand = dynamic_cast<GammaCorrectionCommand*>(command)) {
        // 伽马变换命令
        m_gammaValue = gammaCommand->gamma();
        m_gammaValueSlider->setValue(m_gammaValue * 10);
        gammaValueLabel->setText(QString::number(m_gammaValue, 'f', 1));
        
        // 展开伽马变换参数控件
        if (gammaCollapsible) gammaCollapsible->setCollapsed(false);
        
    } else if (EdgeDetectionCommand *edgeCommand = dynamic_cast<EdgeDetectionCommand*>(command)) {
        // 边缘检测命令
        m_edgeThreshold = edgeCommand->threshold();
        m_edgeThresholdSlider->setValue(m_edgeThreshold);
        edgeValueLabel->setText(QString::number(m_edgeThreshold));
        
        // 展开边缘检测参数控件
        if (edgeCollapsible) edgeCollapsible->setCollapsed(false);
    }
    
    // 更新文件信息（特别是分辨率）
    FileViewSubWindow *currentWin = currentImageSubWindow();
    if (currentWin) {
        updateFileInfo(currentWin);
    }
}

// 应用现代样式
void MainWindow::applyModernStyle()
{
    // 应用现代样式
    QString appStyle = R"(
        /* DockWidget样式 */
        QDockWidget {
            border: 1px solid #d0d0d0;
            border-radius: 4px;
            background-color: #f8f9fa;
        }
        
        QDockWidget::title {
            background-color: #e9ecef;
            padding: 6px;
            border-radius: 4px;
            font-weight: bold;
            color: #495057;
        }
        
        /* GroupBox样式 */
        QGroupBox {
            border: 1px solid #d0d0d0;
            border-radius: 4px;
            margin-top: 10px;
            padding-top: 10px;
            font-weight: bold;
            color: #495057;
            background-color: #ffffff;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            background-color: #ffffff;
        }
        
        /* 标签样式 */
        QLabel {
            color: #495057;
            padding: 2px;
        }
        
        /* 滑块样式 */
        QSlider::groove:horizontal {
            border: 1px solid #d0d0d0;
            height: 4px;
            background: #e9ecef;
            margin: 2px 0;
            border-radius: 2px;
        }
        
        QSlider::handle:horizontal {
            background: #007bff;
            border: 1px solid #0056b3;
            width: 16px;
            height: 16px;
            margin: -6px 0;
            border-radius: 8px;
        }
        
        QSlider::handle:horizontal:hover {
            background: #0056b3;
        }
        
        QSlider::sub-page:horizontal {
            background: #007bff;
            border-radius: 2px;
        }
        
        /* 主窗口样式 */
        QMainWindow {
            background-color: #f8f9fa;
        }
    )";
    
    setStyleSheet(appStyle);
    
    // 设置窗口标题和图标
    setWindowTitle("图像处理工具");
    setWindowIcon(this->style()->standardIcon(QStyle::SP_ComputerIcon));
    
    // 设置窗口最小尺寸
    setMinimumSize(800, 600);
    
    // 设置dockWidget的初始大小
    ui->dockWidget->setMinimumWidth(280);
    ui->dockWidget->setMaximumWidth(350);
}

// 设置主工具栏
void MainWindow::setupMainToolBar()
{
    m_mainToolBar = addToolBar("主工具栏");
    m_mainToolBar->setObjectName("mainToolBar");
    m_mainToolBar->setMovable(true);
    m_mainToolBar->setFloatable(true);
    m_mainToolBar->setIconSize(QSize(24, 24));
    
    // 文件操作
    m_mainToolBar->addAction(ui->actionOpen_O);
    m_mainToolBar->addAction(ui->actionSaveVideo);
    m_mainToolBar->addSeparator();
    
    // 撤销重做
    m_mainToolBar->addAction(ui->action_Z);
    m_mainToolBar->addAction(ui->action_Y);
    m_mainToolBar->addSeparator();
    
    // 图像处理操作
    m_mainToolBar->addAction(ui->action_G);      // 灰度化
    m_mainToolBar->addAction(ui->action_T);      // 二值化
    m_mainToolBar->addAction(ui->action_2);      // 滤波
    m_mainToolBar->addAction(ui->action_3);      // 伽马变换
    m_mainToolBar->addAction(ui->action_4);      // 边缘检测
    
    // 设置工具按钮样式
    for (QAction *action : m_mainToolBar->actions()) {
        if (action->isSeparator()) continue;
        
        // 设置图标（如果没有图标，使用标准图标）
        if (action->icon().isNull()) {
            QString text = action->text();
            if (text.contains("打开")) {
                action->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
            } else if (text.contains("保存")) {
                action->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
            } else if (text.contains("撤销")) {
                action->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
            } else if (text.contains("重做")) {
                action->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
            } else if (text.contains("灰度")) {
                action->setIcon(style()->standardIcon(QStyle::SP_DesktopIcon));
            } else if (text.contains("二值")) {
                action->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
            } else if (text.contains("滤波")) {
                action->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
            } else if (text.contains("伽马")) {
                action->setIcon(style()->standardIcon(QStyle::SP_DriveCDIcon));
            } else if (text.contains("边缘")) {
                action->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
            }
        }
        
        // 设置工具提示
        action->setToolTip(action->text());
    }
}

// 设置参数工具栏（可折叠）
void MainWindow::setupParameterToolBar()
{
    // 获取dockWidget的内容区域
    QWidget *dockContents = ui->dockWidget->widget();
    if (!dockContents) {
        return;
    }
    
    QVBoxLayout *dockLayout = qobject_cast<QVBoxLayout*>(dockContents->layout());
    if (!dockLayout) {
        return;
    }
    
    // 创建参数控制区域
    QGroupBox *parameterGroupBox = new QGroupBox("参数调节", this);
    parameterGroupBox->setObjectName("parameterGroupBox");
    QVBoxLayout *parameterLayout = new QVBoxLayout(parameterGroupBox);
    parameterLayout->setContentsMargins(8, 8, 8, 8);
    parameterLayout->setSpacing(4);
    
    // 创建可折叠控件
    CollapsibleWidget *binaryCollapsible = new CollapsibleWidget("二值化参数", this);
    binaryCollapsible->setObjectName("binaryCollapsible");
    CollapsibleWidget *gammaCollapsible = new CollapsibleWidget("伽马变换参数", this);
    gammaCollapsible->setObjectName("gammaCollapsible");
    CollapsibleWidget *edgeCollapsible = new CollapsibleWidget("边缘检测参数", this);
    edgeCollapsible->setObjectName("edgeCollapsible");
    
    // 创建二值化参数控件
    QWidget *binaryWidget = new QWidget(this);
    QHBoxLayout *binaryLayout = new QHBoxLayout(binaryWidget);
    binaryLayout->setContentsMargins(0, 0, 0, 0);
    binaryLayout->setSpacing(8);
    
    binaryLabel = new QLabel("阈值:", this);
    m_binaryThresholdSlider = new QSlider(Qt::Horizontal, this);
    m_binaryThresholdSlider->setRange(0, 255);
    m_binaryThresholdSlider->setValue(m_binaryThreshold);
    m_binaryThresholdSlider->setFixedWidth(150);
    binaryValueLabel = new QLabel(QString::number(m_binaryThreshold), this);
    binaryValueLabel->setFixedWidth(40);
    binaryValueLabel->setAlignment(Qt::AlignCenter);
    
    binaryLayout->addWidget(binaryLabel);
    binaryLayout->addWidget(m_binaryThresholdSlider);
    binaryLayout->addWidget(binaryValueLabel);
    binaryLayout->addStretch();
    
    // 创建伽马变换参数控件
    QWidget *gammaWidget = new QWidget(this);
    QHBoxLayout *gammaLayout = new QHBoxLayout(gammaWidget);
    gammaLayout->setContentsMargins(0, 0, 0, 0);
    gammaLayout->setSpacing(8);
    
    gammaLabel = new QLabel("伽马值:", this);
    m_gammaValueSlider = new QSlider(Qt::Horizontal, this);
    m_gammaValueSlider->setRange(10, 30); // 1.0-3.0
    m_gammaValueSlider->setValue(m_gammaValue * 10);
    m_gammaValueSlider->setFixedWidth(150);
    gammaValueLabel = new QLabel(QString::number(m_gammaValue, 'f', 1), this);
    gammaValueLabel->setFixedWidth(40);
    gammaValueLabel->setAlignment(Qt::AlignCenter);
    
    gammaLayout->addWidget(gammaLabel);
    gammaLayout->addWidget(m_gammaValueSlider);
    gammaLayout->addWidget(gammaValueLabel);
    gammaLayout->addStretch();
    
    // 创建边缘检测参数控件
    QWidget *edgeWidget = new QWidget(this);
    QHBoxLayout *edgeLayout = new QHBoxLayout(edgeWidget);
    edgeLayout->setContentsMargins(0, 0, 0, 0);
    edgeLayout->setSpacing(8);
    
    edgeLabel = new QLabel("阈值:", this);
    m_edgeThresholdSlider = new QSlider(Qt::Horizontal, this);
    m_edgeThresholdSlider->setRange(0, 200);
    m_edgeThresholdSlider->setValue(m_edgeThreshold);
    m_edgeThresholdSlider->setFixedWidth(150);
    edgeValueLabel = new QLabel(QString::number(m_edgeThreshold), this);
    edgeValueLabel->setFixedWidth(40);
    edgeValueLabel->setAlignment(Qt::AlignCenter);
    
    edgeLayout->addWidget(edgeLabel);
    edgeLayout->addWidget(m_edgeThresholdSlider);
    edgeLayout->addWidget(edgeValueLabel);
    edgeLayout->addStretch();
    
    // 设置可折叠控件内容
    binaryCollapsible->setContent(binaryWidget);
    gammaCollapsible->setContent(gammaWidget);
    edgeCollapsible->setContent(edgeWidget);
    
    // 默认折叠所有参数控件
    binaryCollapsible->setCollapsed(true);
    gammaCollapsible->setCollapsed(true);
    edgeCollapsible->setCollapsed(true);
    
    // 将可折叠控件添加到参数区域
    parameterLayout->addWidget(binaryCollapsible);
    parameterLayout->addWidget(gammaCollapsible);
    parameterLayout->addWidget(edgeCollapsible);
    parameterLayout->addStretch();
    
    // 将参数控制区域添加到dockWidget
    dockLayout->insertWidget(2, parameterGroupBox); // 插入到文件信息和缩放控制之间
    
    // 连接滑块信号
    connect(m_binaryThresholdSlider, &QSlider::valueChanged, this, &MainWindow::on_binaryThresholdSlider_valueChanged);
    connect(m_binaryThresholdSlider, &QSlider::sliderPressed, this, &MainWindow::on_sliderPressed);
    connect(m_binaryThresholdSlider, &QSlider::sliderReleased, this, &MainWindow::on_binaryThresholdSlider_released);
    
    connect(m_gammaValueSlider, &QSlider::valueChanged, this, &MainWindow::on_gammaValueSlider_valueChanged);
    connect(m_gammaValueSlider, &QSlider::sliderPressed, this, &MainWindow::on_sliderPressed);
    connect(m_gammaValueSlider, &QSlider::sliderReleased, this, &MainWindow::on_gammaValueSlider_released);
    
    connect(m_edgeThresholdSlider, &QSlider::valueChanged, this, &MainWindow::on_edgeThresholdSlider_valueChanged);
    connect(m_edgeThresholdSlider, &QSlider::sliderPressed, this, &MainWindow::on_sliderPressed);
    connect(m_edgeThresholdSlider, &QSlider::sliderReleased, this, &MainWindow::on_edgeThresholdSlider_released);
}

// 更新文件信息
void MainWindow::updateFileInfo(FileViewSubWindow *subWindow)
{
    if (!subWindow) {
        // 没有激活窗口时清空信息
        ui->fileNameLabel->setText("文件名：未选择");
        ui->fileSizeLabel->setText("文件大小：--");
        ui->fileTypeLabel->setText("文件类型：--");
        ui->resolutionLabel->setText("分辨率：--");
        return;
    }
    
    // 使用新的getFilePath方法获取文件路径
    QString filePath = subWindow->getFilePath();
    QString fileName = subWindow->windowTitle(); // 使用窗口标题作为显示文件名
    
    if (fileName.isEmpty()) {
        fileName = "未命名文件";
    }
    
    // 文件名显示（限制长度）
    QString displayName = fileName;
    if (displayName.length() > 30) {
        displayName = displayName.left(27) + "...";
    }
    ui->fileNameLabel->setText("文件名：" + displayName);
    
    // 文件大小和类型信息
    if (subWindow->isVideoFile()) {
        // 视频文件处理
        if (!filePath.isEmpty()) {
            QFileInfo fileInfo(filePath);
            qint64 fileSize = fileInfo.size();
            QString sizeStr;
            if (fileSize < 1024) {
                sizeStr = QString("%1 B").arg(fileSize);
            } else if (fileSize < 1024 * 1024) {
                sizeStr = QString("%1 KB").arg(fileSize / 1024.0, 0, 'f', 1);
            } else if (fileSize < 1024 * 1024 * 1024) {
                sizeStr = QString("%1 MB").arg(fileSize / (1024.0 * 1024.0), 0, 'f', 1);
            } else {
                sizeStr = QString("%1 GB").arg(fileSize / (1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
            }
            ui->fileSizeLabel->setText("文件大小：" + sizeStr);
            
            // 文件类型
            QString suffix = fileInfo.suffix().toUpper();
            ui->fileTypeLabel->setText("文件类型：" + suffix + " 视频");
        } else {
            ui->fileSizeLabel->setText("文件大小：视频文件");
            ui->fileTypeLabel->setText("文件类型：视频");
        }
        
        // 视频分辨率
        QImage currentImage = subWindow->getCurrentImage();
        if (!currentImage.isNull()) {
            QString resolution = QString("%1 × %2").arg(currentImage.width()).arg(currentImage.height());
            ui->resolutionLabel->setText("分辨率：" + resolution);
        } else {
            ui->resolutionLabel->setText("分辨率：--");
        }
    } else {
        // 图像文件处理
        if (!filePath.isEmpty()) {
            QFileInfo fileInfo(filePath);
            qint64 fileSize = fileInfo.size();
            QString sizeStr;
            if (fileSize < 1024) {
                sizeStr = QString("%1 B").arg(fileSize);
            } else if (fileSize < 1024 * 1024) {
                sizeStr = QString("%1 KB").arg(fileSize / 1024.0, 0, 'f', 1);
            } else if (fileSize < 1024 * 1024 * 1024) {
                sizeStr = QString("%1 MB").arg(fileSize / (1024.0 * 1024.0), 0, 'f', 1);
            } else {
                sizeStr = QString("%1 GB").arg(fileSize / (1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
            }
            ui->fileSizeLabel->setText("文件大小：" + sizeStr);
            
            // 文件类型
            QString suffix = fileInfo.suffix().toUpper();
            QString typeStr;
            if (suffix == "JPG" || suffix == "JPEG") {
                typeStr = "JPEG 图像";
            } else if (suffix == "PNG") {
                typeStr = "PNG 图像";
            } else if (suffix == "BMP") {
                typeStr = "BMP 图像";
            } else {
                typeStr = suffix + " 文件";
            }
            ui->fileTypeLabel->setText("文件类型：" + typeStr);
        } else {
            // 如果没有文件路径，使用图像数据估算
            QImage currentImage = subWindow->getCurrentImage();
            if (!currentImage.isNull()) {
                qint64 estimatedSize = currentImage.sizeInBytes();
                QString sizeStr;
                if (estimatedSize < 1024) {
                    sizeStr = QString("%1 B").arg(estimatedSize);
                } else if (estimatedSize < 1024 * 1024) {
                    sizeStr = QString("%1 KB").arg(estimatedSize / 1024.0, 0, 'f', 1);
                } else if (estimatedSize < 1024 * 1024 * 1024) {
                    sizeStr = QString("%1 MB").arg(estimatedSize / (1024.0 * 1024.0), 0, 'f', 1);
                } else {
                    sizeStr = QString("%1 GB").arg(estimatedSize / (1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
                }
                ui->fileSizeLabel->setText("文件大小：≈" + sizeStr);
                
                QString formatStr;
                switch (currentImage.format()) {
                    case QImage::Format_RGB32:
                    case QImage::Format_ARGB32:
                        formatStr = "RGB 图像";
                        break;
                    case QImage::Format_Grayscale8:
                        formatStr = "灰度图像";
                        break;
                    default:
                        formatStr = "图像文件";
                        break;
                }
                ui->fileTypeLabel->setText("文件类型：" + formatStr);
            } else {
                ui->fileSizeLabel->setText("文件大小：--");
                ui->fileTypeLabel->setText("文件类型：--");
            }
        }
        
        // 分辨率
        QImage currentImage = subWindow->getCurrentImage();
        if (!currentImage.isNull()) {
            QString resolution = QString("%1 × %2").arg(currentImage.width()).arg(currentImage.height());
            ui->resolutionLabel->setText("分辨率：" + resolution);
        } else {
            ui->resolutionLabel->setText("分辨率：--");
        }
    }
}

