#include "usernameedit.h"

#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

UsernameEdit::UsernameEdit(QWidget *parent) : QLineEdit(parent), m_hasFocus(false)
{
    setFixedHeight(41);
    setStyleSheet("border: none; background: transparent;");
}

void UsernameEdit::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);//告诉编译器参数未使用，消除警告
    QPainter painter(this);//创建画家对象，绑定到当前控件
    painter.setRenderHint(QPainter::Antialiasing);//启用抗锯齿，让边缘更平滑

    int penWidth = 2;
    int halfPen = penWidth / 2;

    // 1. 先绘制背景
    QPainterPath bgPath;
    bgPath.addRoundedRect(rect(), 8, 8);//创建圆角矩形路径（半径 8px）
    painter.setClipPath(bgPath);//设置裁剪路径 - 后续所有绘制只在这个形状内生效

    QColor bgColor = m_hasFocus ? QColor(0xE3F2FD) : QColor(0xF5F5F5);//焦点状态选择颜色
    painter.fillPath(bgPath, bgColor);//填充背景色

    // 2. 再绘制边框（在裁剪路径内）
    QPen borderPen(m_hasFocus ? QColor(0x2196F3) : QColor(0xE0E0E0));
    borderPen.setWidth(penWidth);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);//不填充，只绘制边框
    painter.drawRoundedRect(rect().adjusted(halfPen, halfPen, -halfPen, -halfPen), 8, 8);//将矩形向内收缩 1px
    /*
        为什么收缩 1px？
        画笔宽度 2px，一半在线内一半在线外
        向内收缩 1px 确保边框完全在控件区域内
        不会被控件边界裁剪
    */

    // 3. 绘制图标
    painter.setClipRect(rect()); // setClipRect(rect()) - 重置裁剪区域为整个控件
    QPixmap icon(":/icons/icon-person");
    if (!icon.isNull()) {
        painter.setRenderHint(QPainter::SmoothPixmapTransform);//启用图片缩放抗锯齿
        painter.drawPixmap(12, (height() - 20) / 2, 20, 20, icon);//将图标绘制到：x=12, y=垂直居中,宽高=20x20
    }

    // 4. 绘制文本
    painter.setPen(m_hasFocus ? QColor(0x333) : QColor(0x999));
    QFont font("Microsoft YaHei", 12);//设置字体：微软雅黑，12pt
    painter.setFont(font);
    QRect textRect = rect().adjusted(42, 0, -10, 0);//文本区域从 x=42 开始（图标右侧），右边距 10px
    painter.drawText(textRect, Qt::AlignVCenter, text());//Qt::AlignVCenter - 垂直居中对齐
}

void UsernameEdit::focusInEvent(QFocusEvent *event)
{
    m_hasFocus = true;
    QLineEdit::focusInEvent(event);
    update();
}

void UsernameEdit::focusOutEvent(QFocusEvent *event)
{
    m_hasFocus = false;
    QLineEdit::focusOutEvent(event);
    update();
}
/*
如果不调用父类，输入框的默认行为会丢失：
光标不会显示
无法输入文本
文本选择功能失效
调用父类确保基本功能正常，同时我们添加自定义的视觉效果。
*/

/*
整体工作流程
┌─────────────────────────────────────────┐
│  1. 创建圆角矩形裁剪路径                  │
│  2. 填充背景色（在裁剪区域内）             │
│  3. 绘制边框（向内收缩1px，不被裁剪）      │
│  4. 重置裁剪，绘制图标                     │
│  5. 绘制文本                              │
└─────────────────────────────────────────┘
关键设计：
先背景后边框，避免边框被覆盖
边框收缩绘制，确保完整显示
裁剪路径确保圆角效果
图标和文本在裁剪外绘制，不受影响
*/