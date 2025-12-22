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
#include <QtWidgets/QGroupBox>
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
    QAction *action_Z;
    QAction *action_Y;
    QAction *action_G;
    QAction *action_T;
    QAction *action_2;
    QAction *action_3;
    QAction *action_4;
    QAction *actionSaveVideo;
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
    QGroupBox *fileInfoGroupBox;
    QVBoxLayout *fileInfoLayout;
    QLabel *fileNameLabel;
    QLabel *fileSizeLabel;
    QLabel *fileTypeLabel;
    QLabel *resolutionLabel;
    QGroupBox *scaleGroupBox;
    QVBoxLayout *scaleLayout;
    QLabel *labelScale;
    QSlider *horizontalSliderScale;
    QSpacerItem *verticalSpacer;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(947, 604);
        actionNew_new = new QAction(MainWindow);
        actionNew_new->setObjectName("actionNew_new");
        actionOpen_O = new QAction(MainWindow);
        actionOpen_O->setObjectName("actionOpen_O");
        action_Z = new QAction(MainWindow);
        action_Z->setObjectName("action_Z");
        action_Y = new QAction(MainWindow);
        action_Y->setObjectName("action_Y");
        action_G = new QAction(MainWindow);
        action_G->setObjectName("action_G");
        action_T = new QAction(MainWindow);
        action_T->setObjectName("action_T");
        action_2 = new QAction(MainWindow);
        action_2->setObjectName("action_2");
        action_3 = new QAction(MainWindow);
        action_3->setObjectName("action_3");
        action_4 = new QAction(MainWindow);
        action_4->setObjectName("action_4");
        actionSaveVideo = new QAction(MainWindow);
        actionSaveVideo->setObjectName("actionSaveVideo");
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
        fileInfoGroupBox = new QGroupBox(dockWidgetContents);
        fileInfoGroupBox->setObjectName("fileInfoGroupBox");
        fileInfoLayout = new QVBoxLayout(fileInfoGroupBox);
        fileInfoLayout->setObjectName("fileInfoLayout");
        fileNameLabel = new QLabel(fileInfoGroupBox);
        fileNameLabel->setObjectName("fileNameLabel");
        fileNameLabel->setWordWrap(true);

        fileInfoLayout->addWidget(fileNameLabel);

        fileSizeLabel = new QLabel(fileInfoGroupBox);
        fileSizeLabel->setObjectName("fileSizeLabel");

        fileInfoLayout->addWidget(fileSizeLabel);

        fileTypeLabel = new QLabel(fileInfoGroupBox);
        fileTypeLabel->setObjectName("fileTypeLabel");

        fileInfoLayout->addWidget(fileTypeLabel);

        resolutionLabel = new QLabel(fileInfoGroupBox);
        resolutionLabel->setObjectName("resolutionLabel");

        fileInfoLayout->addWidget(resolutionLabel);


        verticalLayout->addWidget(fileInfoGroupBox);

        scaleGroupBox = new QGroupBox(dockWidgetContents);
        scaleGroupBox->setObjectName("scaleGroupBox");
        scaleLayout = new QVBoxLayout(scaleGroupBox);
        scaleLayout->setObjectName("scaleLayout");
        labelScale = new QLabel(scaleGroupBox);
        labelScale->setObjectName("labelScale");

        scaleLayout->addWidget(labelScale);

        horizontalSliderScale = new QSlider(scaleGroupBox);
        horizontalSliderScale->setObjectName("horizontalSliderScale");
        horizontalSliderScale->setMinimum(1);
        horizontalSliderScale->setMaximum(500);
        horizontalSliderScale->setSingleStep(15);
        horizontalSliderScale->setValue(100);
        horizontalSliderScale->setOrientation(Qt::Orientation::Horizontal);
        horizontalSliderScale->setTickPosition(QSlider::TickPosition::TicksBelow);
        horizontalSliderScale->setTickInterval(50);

        scaleLayout->addWidget(horizontalSliderScale);


        verticalLayout->addWidget(scaleGroupBox);

        verticalSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        dockWidget->setWidget(dockWidgetContents);
        MainWindow->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, dockWidget);

        menubar->addAction(menu_F->menuAction());
        menubar->addAction(menu_E->menuAction());
        menu_F->addAction(actionNew_new);
        menu_F->addAction(actionOpen_O);
        menu_E->addAction(action_Z);
        menu_E->addAction(action_Y);
        menu_E->addSeparator();
        menu_E->addAction(action_G);
        menu_E->addAction(action_T);
        menu_E->addAction(action_2);
        menu_E->addAction(action_3);
        menu_E->addAction(action_4);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        actionNew_new->setText(QCoreApplication::translate("MainWindow", "New(&N)", nullptr));
        actionOpen_O->setText(QCoreApplication::translate("MainWindow", "Open(&O)", nullptr));
        action_Z->setText(QCoreApplication::translate("MainWindow", "\346\222\244\351\224\200(&Z)", nullptr));
        action_Y->setText(QCoreApplication::translate("MainWindow", "\351\207\215\345\201\232(&Y)", nullptr));
        action_G->setText(QCoreApplication::translate("MainWindow", "\347\201\260\345\272\246\345\214\226(&G)", nullptr));
        action_T->setText(QCoreApplication::translate("MainWindow", "\344\272\214\345\200\274\345\214\226(&T)", nullptr));
        action_2->setText(QCoreApplication::translate("MainWindow", "\346\273\244\346\263\242", nullptr));
        action_3->setText(QCoreApplication::translate("MainWindow", "\344\274\275\351\251\254\345\217\230\346\215\242", nullptr));
        action_4->setText(QCoreApplication::translate("MainWindow", "\350\276\271\347\274\230\346\243\200\346\265\213", nullptr));
        actionSaveVideo->setText(QCoreApplication::translate("MainWindow", "\344\277\235\345\255\230\350\247\206\351\242\221", nullptr));
        menu_F->setTitle(QCoreApplication::translate("MainWindow", "\346\226\207\344\273\266(&F)", nullptr));
        menu_E->setTitle(QCoreApplication::translate("MainWindow", "\347\274\226\350\276\221(&E)", nullptr));
        dockWidget->setWindowTitle(QCoreApplication::translate("MainWindow", "\346\216\247\345\210\266\351\235\242\346\235\277", nullptr));
        fileInfoGroupBox->setTitle(QCoreApplication::translate("MainWindow", "\346\226\207\344\273\266\344\277\241\346\201\257", nullptr));
        fileNameLabel->setText(QCoreApplication::translate("MainWindow", "\346\226\207\344\273\266\345\220\215\357\274\232\346\234\252\351\200\211\346\213\251", nullptr));
        fileSizeLabel->setText(QCoreApplication::translate("MainWindow", "\346\226\207\344\273\266\345\244\247\345\260\217\357\274\232--", nullptr));
        fileTypeLabel->setText(QCoreApplication::translate("MainWindow", "\346\226\207\344\273\266\347\261\273\345\236\213\357\274\232--", nullptr));
        resolutionLabel->setText(QCoreApplication::translate("MainWindow", "\345\210\206\350\276\250\347\216\207\357\274\232--", nullptr));
        scaleGroupBox->setTitle(QCoreApplication::translate("MainWindow", "\347\274\251\346\224\276\346\216\247\345\210\266", nullptr));
        labelScale->setText(QCoreApplication::translate("MainWindow", "\345\275\223\345\211\215\347\274\251\346\224\276\357\274\232100%", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
