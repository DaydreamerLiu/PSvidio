#include "collapsiblewidget.h"

CollapsibleWidget::CollapsibleWidget(const QString &title, QWidget *parent)
    : QWidget(parent)
    , m_collapsed(false)
    , m_contentHeight(0)
    , m_animationDuration(200)
{
    // 创建切换按钮
    m_toggleButton = new QToolButton(this);
    m_toggleButton->setText(title);
    m_toggleButton->setCheckable(true);
    m_toggleButton->setChecked(false);
    m_toggleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_toggleButton->setArrowType(Qt::RightArrow);
    m_toggleButton->setStyleSheet(R"(
        QToolButton {
            background-color: transparent;
            border: none;
            border-radius: 4px;
            padding: 6px 8px;
            text-align: left;
            font-weight: 500;
            font-size: 13px;
        }
        QToolButton:hover {
            background-color: #f5f5f5;
        }
        QToolButton:pressed {
            background-color: #e0e0e0;
        }
    )");
    
    // 创建内容区域
    m_contentWidget = new QWidget(this);
    m_contentWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_contentWidget->setMaximumHeight(0);
    m_contentWidget->setMinimumHeight(0);
    
    // 创建布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_toggleButton);
    m_mainLayout->addWidget(m_contentWidget);
    
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(16, 8, 16, 8);
    m_contentLayout->setSpacing(8);
    
    // 连接信号
    connect(m_toggleButton, &QToolButton::clicked, this, &CollapsibleWidget::toggleCollapsed);
    
    // 设置样式
    setStyleSheet(R"(
        CollapsibleWidget {
            background-color: #ffffff;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            margin: 2px 0px;
        }
    )");
}

CollapsibleWidget::~CollapsibleWidget()
{
}

void CollapsibleWidget::setContent(QWidget *widget)
{
    if (widget) {
        m_contentLayout->addWidget(widget);
        m_contentHeight = calculateContentHeight();
        if (!m_collapsed) {
            m_contentWidget->setMaximumHeight(m_contentHeight);
            m_contentWidget->setMinimumHeight(m_contentHeight);
        }
    }
}

void CollapsibleWidget::setTitle(const QString &title)
{
    m_toggleButton->setText(title);
}

void CollapsibleWidget::setCollapsed(bool collapsed)
{
    if (m_collapsed != collapsed) {
        m_collapsed = collapsed;
        m_toggleButton->setChecked(!collapsed);
        updateArrow();
        
        if (m_collapsed) {
            m_contentWidget->setMaximumHeight(0);
            m_contentWidget->setMinimumHeight(0);
        } else {
            m_contentHeight = calculateContentHeight();
            m_contentWidget->setMaximumHeight(m_contentHeight);
            m_contentWidget->setMinimumHeight(m_contentHeight);
        }
        
        emit collapsedChanged(m_collapsed);
    }
}

bool CollapsibleWidget::isCollapsed() const
{
    return m_collapsed;
}

int CollapsibleWidget::contentHeight() const
{
    return m_contentHeight;
}

void CollapsibleWidget::setContentHeight(int height)
{
    m_contentHeight = height;
    if (!m_collapsed) {
        m_contentWidget->setMaximumHeight(height);
        m_contentWidget->setMinimumHeight(height);
    }
}

void CollapsibleWidget::toggleCollapsed()
{
    setCollapsed(!m_collapsed);
}

void CollapsibleWidget::updateArrow()
{
    m_toggleButton->setArrowType(m_collapsed ? Qt::RightArrow : Qt::DownArrow);
}

int CollapsibleWidget::calculateContentHeight() const
{
    if (m_contentLayout->count() == 0) {
        return 0;
    }
    
    int height = 0;
    for (int i = 0; i < m_contentLayout->count(); ++i) {
        QLayoutItem *item = m_contentLayout->itemAt(i);
        if (item && item->widget()) {
            height += item->widget()->sizeHint().height();
        }
    }
    
    // 添加边距
    height += m_contentLayout->contentsMargins().top() + m_contentLayout->contentsMargins().bottom();
    height += (m_contentLayout->count() - 1) * m_contentLayout->spacing();
    
    return height;
}