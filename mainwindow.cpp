#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fileviewsubwindow.h" // 包含自定义子窗口头文件
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. 设置标签页模式（核心：支持多文件标签切换）
    ui->mdiArea->setViewMode(QMdiArea::TabbedView);


    // 3. 标签栏优化（Qt6 交互体验增强）
    ui->mdiArea->setTabsClosable(true);    // 标签带关闭按钮
    ui->mdiArea->setTabsMovable(true);     // 标签可拖动排序
    ui->mdiArea->setTabPosition(QTabWidget::North);  // 标签在顶部（Qt6 正确枚举）
    ui->mdiArea->setTabShape(QTabWidget::Rounded);   // 圆角标签（Qt6 美化）

    // 4. MDI窗口行为优化
    ui->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    // 打印Qt版本（调试用，Qt6.9.2 宏QT_VERSION_STR）
    qDebug() << "Qt 版本：" << QT_VERSION_STR;
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

