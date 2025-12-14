#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    // 关键：启用Qt6高DPI缩放（必须在QApplication创建前）
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication a(argc, argv);

    MainWindow w;
    w.show();
    return a.exec();
}
