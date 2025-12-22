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
    void on_actionSaveVideo_triggered();

    void on_horizontalSliderScale_valueChanged(int value);

    void on_mdiArea_subWindowActivated(QMdiSubWindow *arg1);

    // 图像处理相关槽函数
    void on_action_G_triggered();
    void on_action_T_triggered();
    void on_action_2_triggered();
    void on_action_3_triggered();
    void on_action_4_triggered();
    void on_action_Mosaic_triggered();

    // 撤销重做相关槽函数
    void on_action_Z_triggered();
    void on_action_Y_triggered();

    // 槽函数
    void on_sliderPressed();
    void on_binaryThresholdSlider_valueChanged(int value);
    void on_binaryThresholdSlider_released();
    void on_gammaValueSlider_valueChanged(int value);
    void on_gammaValueSlider_released();
    void on_edgeThresholdSlider_valueChanged(int value);
    void on_edgeThresholdSlider_released();
    void onCommandApplied(ImageCommand *command); // 处理命令应用信号

private:
    Ui::MainWindow *ui;
    // 获取当前激活的图片子窗口（过滤视频窗口）
    FileViewSubWindow* currentImageSubWindow();

    // 工具栏滑块控件
    QSlider *m_binaryThresholdSlider;
    QSlider *m_gammaValueSlider;
    QSlider *m_edgeThresholdSlider;
    // 滑块标签控件
    QLabel *binaryLabel; // 二值化阈值标签
    QLabel *gammaLabel; // 伽马值标签
    QLabel *edgeLabel; // 边缘检测阈值标签
    // 滑块数值显示标签
    QLabel *binaryValueLabel; // 二值化阈值数值显示
    QLabel *gammaValueLabel; // 伽马值数值显示
    QLabel *edgeValueLabel; // 边缘检测阈值数值显示
    // 滑块当前值
    int m_binaryThreshold;
    double m_gammaValue;
    int m_edgeThreshold;
    // 用于延迟处理的定时器
    QTimer *m_timer; // 用于滑块停止拖动后延迟处理
    
    // 工具栏
    QToolBar *m_mainToolBar;
    QToolBar *m_parameterToolBar;
    
    // 私有方法
    void applyModernStyle();
    void setupMainToolBar();
    void setupParameterToolBar();
    void createSliderControl(QLabel* &label, QSlider* &slider, QLabel* &valueLabel, 
                           const QString& title, int min, int max, int value, 
                           const QString& tooltip, const char* valueChangedSlot,
                           const char* sliderPressedSlot, const char* sliderReleasedSlot);
};
#endif // MAINWINDOW_H
