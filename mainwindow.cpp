#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fileviewsubwindow.h" // 包含自定义子窗口头文件
#include "grayscalecommand.h"
#include "binarycommand.h"
#include "meanfiltercommand.h"
#include "gammacorrectioncommand.h"
#include "edgedetectioncommand.h"
#include <QFileDialog>
#include <QToolBar>
#include <QLabel>

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

    // 5. 添加工具栏滑块控件
    // 创建一个新的工具栏用于放置滑块控件
    QToolBar *toolBar = addToolBar("ImageToolBar");
    toolBar->setVisible(true); // 确保工具栏默认可见
    
    // 二值化阈值滑块
    m_binaryThresholdSlider = new QSlider(Qt::Horizontal, this);
    m_binaryThresholdSlider->setRange(0, 255);
    m_binaryThresholdSlider->setValue(m_binaryThreshold);
    m_binaryThresholdSlider->setToolTip("二值化阈值 (0-255)");
    m_binaryThresholdSlider->setVisible(false); // 默认隐藏
    connect(m_binaryThresholdSlider, &QSlider::valueChanged, this, &MainWindow::on_binaryThresholdSlider_valueChanged);
    toolBar->addWidget(new QLabel("二值化阈值：", this));
    toolBar->addWidget(m_binaryThresholdSlider);

    // 伽马变换值滑块
    m_gammaValueSlider = new QSlider(Qt::Horizontal, this);
    m_gammaValueSlider->setRange(1, 30); // 1-30表示1.0-3.0
    m_gammaValueSlider->setValue(m_gammaValue * 10);
    m_gammaValueSlider->setToolTip("伽马值 (1.0-3.0)");
    m_gammaValueSlider->setVisible(false); // 默认隐藏
    connect(m_gammaValueSlider, &QSlider::valueChanged, this, &MainWindow::on_gammaValueSlider_valueChanged);
    toolBar->addWidget(new QLabel("伽马值：", this));
    toolBar->addWidget(m_gammaValueSlider);

    // 边缘检测阈值滑块
    m_edgeThresholdSlider = new QSlider(Qt::Horizontal, this);
    m_edgeThresholdSlider->setRange(0, 200); // 0-200的阈值范围
    m_edgeThresholdSlider->setValue(m_edgeThreshold);
    m_edgeThresholdSlider->setToolTip("边缘检测阈值 (0-200)");
    m_edgeThresholdSlider->setVisible(false); // 默认隐藏
    connect(m_edgeThresholdSlider, &QSlider::valueChanged, this, &MainWindow::on_edgeThresholdSlider_valueChanged);
    toolBar->addWidget(new QLabel("边缘阈值：", this));
    toolBar->addWidget(m_edgeThresholdSlider);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 获取当前激活的图片子窗口（过滤视频窗口）
FileViewSubWindow* MainWindow::currentImageSubWindow()
{
    QMdiSubWindow *activeWin = ui->mdiArea->activeSubWindow();
    if (!activeWin) return nullptr;
    // 转换为自定义子窗口类型
    return qobject_cast<FileViewSubWindow*>(activeWin->widget()->parentWidget());
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

    // 隐藏其他滑块控件
    m_binaryThresholdSlider->setVisible(false);
    m_gammaValueSlider->setVisible(false);

    // 创建并应用灰度化命令
    imageWin->applyImageCommand(new GrayscaleCommand(imageWin->getCurrentImage()));
}

// 二值化
void MainWindow::on_action_T_triggered()
{
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;

    // 显示二值化阈值滑块，隐藏其他滑块
    m_binaryThresholdSlider->setVisible(true);
    m_gammaValueSlider->setVisible(false);

    // 创建并应用二值化命令
    imageWin->applyImageCommand(new BinaryCommand(imageWin->getCurrentImage(), m_binaryThreshold));
}

// 3×3均值滤波
void MainWindow::on_action_2_triggered()
{
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;

    // 隐藏所有滑块控件
    m_binaryThresholdSlider->setVisible(false);
    m_gammaValueSlider->setVisible(false);

    // 创建并应用均值滤波命令
    imageWin->applyImageCommand(new MeanFilterCommand(imageWin->getCurrentImage()));
}

// 伽马变换
void MainWindow::on_action_3_triggered()
{
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;

    // 显示伽马值滑块，隐藏其他滑块
    m_binaryThresholdSlider->setVisible(false);
    m_gammaValueSlider->setVisible(true);

    // 创建并应用伽马变换命令
    imageWin->applyImageCommand(new GammaCorrectionCommand(imageWin->getCurrentImage(), m_gammaValue));
}

// 边缘检测
void MainWindow::on_action_4_triggered()
{
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;

    // 显示边缘检测阈值滑块，隐藏其他滑块
    m_binaryThresholdSlider->setVisible(false);
    m_gammaValueSlider->setVisible(false);
    m_edgeThresholdSlider->setVisible(true);

    // 创建并应用边缘检测命令
    imageWin->applyImageCommand(new EdgeDetectionCommand(imageWin->getCurrentImage(), m_edgeThreshold));
}

// 撤销
void MainWindow::on_action_Z_triggered()
{
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;

    imageWin->undo();
}

// 重做
void MainWindow::on_action_Y_triggered()
{
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;

    imageWin->redo();
}

// 二值化阈值滑块变化
void MainWindow::on_binaryThresholdSlider_valueChanged(int value)
{
    m_binaryThreshold = value;
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;

    // 重新应用二值化命令
    imageWin->applyImageCommand(new BinaryCommand(imageWin->getCurrentImage(), m_binaryThreshold));
}

// 伽马值滑块变化
void MainWindow::on_gammaValueSlider_valueChanged(int value)
{
    m_gammaValue = value / 10.0;
    
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;
    
    imageWin->applyImageCommand(new GammaCorrectionCommand(imageWin->getCurrentImage(), m_gammaValue));
}

void MainWindow::on_edgeThresholdSlider_valueChanged(int value)
{
    m_edgeThreshold = value;
    
    FileViewSubWindow *imageWin = currentImageSubWindow();
    if (!imageWin) return;
    
    imageWin->applyImageCommand(new EdgeDetectionCommand(imageWin->getCurrentImage(), m_edgeThreshold));
}

