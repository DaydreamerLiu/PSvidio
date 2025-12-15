/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionNew_new;
    QAction *actionOpen_O;
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QMdiArea *mdiArea;
    QMenuBar *menubar;
    QMenu *menu_F;
    QMenu *menu_E;
    QStatusBar *statusbar;
    QDockWidget *dockWidget;
    QWidget *dockWidgetContents;
    QVBoxLayout *verticalLayout;
    QSpacerItem *verticalSpacer;
    QLabel *labelScale;
    QSlider *horizontalSliderScale;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(947, 604);
        actionNew_new = new QAction(MainWindow);
        actionNew_new->setObjectName("actionNew_new");
        actionOpen_O = new QAction(MainWindow);
        actionOpen_O->setObjectName("actionOpen_O");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName("gridLayout");
        mdiArea = new QMdiArea(centralwidget);
        mdiArea->setObjectName("mdiArea");

        gridLayout->addWidget(mdiArea, 0, 0, 1, 1);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 947, 19));
        menu_F = new QMenu(menubar);
        menu_F->setObjectName("menu_F");
        menu_E = new QMenu(menubar);
        menu_E->setObjectName("menu_E");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);
        dockWidget = new QDockWidget(MainWindow);
        dockWidget->setObjectName("dockWidget");
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName("dockWidgetContents");
        verticalLayout = new QVBoxLayout(dockWidgetContents);
        verticalLayout->setObjectName("verticalLayout");
        verticalSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        labelScale = new QLabel(dockWidgetContents);
        labelScale->setObjectName("labelScale");

        verticalLayout->addWidget(labelScale);

        horizontalSliderScale = new QSlider(dockWidgetContents);
        horizontalSliderScale->setObjectName("horizontalSliderScale");
        horizontalSliderScale->setMinimum(1);
        horizontalSliderScale->setMaximum(500);
        horizontalSliderScale->setSingleStep(15);
        horizontalSliderScale->setValue(100);
        horizontalSliderScale->setOrientation(Qt::Orientation::Horizontal);
        horizontalSliderScale->setTickPosition(QSlider::TickPosition::TicksBelow);
        horizontalSliderScale->setTickInterval(50);

        verticalLayout->addWidget(horizontalSliderScale);

        dockWidget->setWidget(dockWidgetContents);
        MainWindow->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, dockWidget);

        menubar->addAction(menu_F->menuAction());
        menubar->addAction(menu_E->menuAction());
        menu_F->addAction(actionNew_new);
        menu_F->addAction(actionOpen_O);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        actionNew_new->setText(QCoreApplication::translate("MainWindow", "New(&N)", nullptr));
        actionOpen_O->setText(QCoreApplication::translate("MainWindow", "Open(&O)", nullptr));
        menu_F->setTitle(QCoreApplication::translate("MainWindow", "\346\226\207\344\273\266(&F)", nullptr));
        menu_E->setTitle(QCoreApplication::translate("MainWindow", "\347\274\226\350\276\221(&E)", nullptr));
        dockWidget->setWindowTitle(QCoreApplication::translate("MainWindow", "\345\267\245\345\205\267\347\256\261", nullptr));
        labelScale->setText(QCoreApplication::translate("MainWindow", "\345\275\223\345\211\215\347\274\251\346\224\276\357\274\232100%", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
