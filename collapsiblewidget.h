#ifndef COLLAPSIBLEWIDGET_H
#define COLLAPSIBLEWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QScrollArea>
#include <QToolButton>
#include <QStyle>

class CollapsibleWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int contentHeight READ contentHeight WRITE setContentHeight)

public:
    explicit CollapsibleWidget(const QString &title = "", QWidget *parent = nullptr);
    ~CollapsibleWidget();

    void setContent(QWidget *widget);
    void setTitle(const QString &title);
    void setCollapsed(bool collapsed);
    bool isCollapsed() const;

    int contentHeight() const;
    void setContentHeight(int height);

signals:
    void collapsedChanged(bool collapsed);

private slots:
    void toggleCollapsed();

private:
    void updateArrow();
    int calculateContentHeight() const;

    QToolButton *m_toggleButton;
    QWidget *m_contentWidget;
    QVBoxLayout *m_mainLayout;
    QVBoxLayout *m_contentLayout;
    QPropertyAnimation *m_contentAnimation;
    QParallelAnimationGroup *m_animationGroup;
    
    bool m_collapsed;
    int m_contentHeight;
    int m_animationDuration;
};

#endif // COLLAPSIBLEWIDGET_H