#include "iconlineedit.h"
#include <QPainter>
#include <QPaintEvent>

IconLineEdit::IconLineEdit(QWidget *parent)
    : QLineEdit(parent)
    , m_iconSize(24, 24)    // 默认图标大小 18x18
    , m_iconSpacing(6)       // A：图标与左边框的距离
    , m_textSpacing(3)       // B：图标与文字的距离
{
    // TODO 1: 初始化文字边距
    // 目标：调用 updateTextMargins() 让文字不会覆盖到图标区域
    // 思考：为什么放在构造函数里调用？
    updateTextMargins();
}

void IconLineEdit::setIcon(const QIcon &icon)
{
    // TODO 2: 保存图标并触发重绘
    // 目标：
    //   1. 把 icon 存到 m_icon
    //   2. 通知 Qt "我需要重绘" → 调用哪个方法？
    //
    // 思考：如果只存 m_icon 不调用重绘方法，会发生什么？
    m_icon = icon;
    update();
}

void IconLineEdit::setIconPath(const QString &path)
{
    // 这是一个便捷方法，已帮你写好
    // 学习点：QIcon 可以从资源路径直接加载 SVG/PNG
    setIcon(QIcon(path));
}

void IconLineEdit::setIconSize(const QSize &size)
{
    // TODO 3: 更新图标大小
    // 目标：
    //   1. 保存新尺寸
    //   2. 更新文字边距（因为图标变大了）
    //   3. 触发重绘
    //
    // 思考：为什么改图标大小会影响文字边距？
    m_iconSize = size;
    updateTextMargins();
    update();
}

void IconLineEdit::setIconSpacing(int spacing)
{
    // TODO 4: 更新间距
    // 目标：同 TODO 3 的模式，保存→更新边距→重绘
    m_iconSpacing = spacing;
    updateTextMargins();
    update();
}

int IconLineEdit::iconSpacing() const
{
    return m_iconSpacing;
}

void IconLineEdit::setTextSpacing(int spacing)
{
    m_textSpacing = spacing;
    updateTextMargins();
    update();
}

int IconLineEdit::textSpacing() const
{
    return m_textSpacing;
}

void IconLineEdit::paintEvent(QPaintEvent *event)
{
    // TODO 5: ★ 核心方法 ★
    // 任务流程：
    //
    // 第1步：先让 QLineEdit 完成它自己的绘制（背景、边框、文字、光标）
    //   提示：调用父类的 paintEvent，传入 event 参数

    QLineEdit::paintEvent(event);

    //
    // 第2步：如果 m_icon 为空，直接 return（没有图标可画）
    //   提示：用 m_icon.isNull() 判断
    //

    if(m_icon.isNull()){
        return;
    }

    // 第3步：创建 QPainter 对象
    //   提示：QPainter painter(this);

    QPainter painter(this);

    //
    // 第4步：开启抗锯齿（让图标边缘更平滑）
    //   提示：painter.setRenderHint(QPainter::Antialiasing);
    //

    painter.setRenderHint(QPainter::Antialiasing);

    // 第5步：计算图标绘制区域 QRect
    //   - X 起点 = m_iconSpacing（距离左边框的间距）
    //   - Y 起点 = (height() - m_iconSize.height()) / 2（垂直居中）
    //   - 宽度 = m_iconSize.width()
    //   - 高度 = m_iconSize.height()
    //

    int x = m_iconSpacing;
    int y = (height() - m_iconSize.height()) / 2;
    QRect iconRect(x, y, m_iconSize.width(), m_iconSize.height());

    // 第6步：用 QIcon::paint() 绘制图标
    //   提示：m_icon.paint(&painter, iconRect);
    //

    m_icon.paint(&painter, iconRect);

    // 思考题：
    //   - 第1步如果放在第6步之后调用，会有什么效果？
    //   - QPainter 为什么不需要显式"关闭"或"释放"？
}

void IconLineEdit::updateTextMargins()
{
    // TODO 6: 计算并设置左侧文字边距
    // 目标：
    //   左边距 = 间距 + 图标宽度 + 间距  (spacing + icon + spacing)
    //   调用 setTextMargins(左边距, 0, 0, 0)
    //
    // 思考：为什么只需要设置左边距？上下右边距为什么是 0？

    int leftMargin = m_iconSpacing + m_iconSize.width() + m_textSpacing;

    // setTextMargins(左, 上, 右, 下)
    setTextMargins(leftMargin, 0, 0, 0);
}
