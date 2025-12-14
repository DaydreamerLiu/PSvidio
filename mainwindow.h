#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMdiSubWindow>
#include "fileviewsubwindow.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionNew_new_triggered();

    void on_actionOpen_O_triggered();

    void on_horizontalSliderScale_valueChanged(int value);

    void on_mdiArea_subWindowActivated(QMdiSubWindow *arg1);

private:
    Ui::MainWindow *ui;
    // 获取当前激活的图片子窗口（过滤视频窗口）
    FileViewSubWindow* currentImageSubWindow();
};
#endif // MAINWINDOW_H
