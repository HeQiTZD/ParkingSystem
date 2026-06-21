#ifndef CIRCLEPROGRESS_H
#define CIRCLEPROGRESS_H

#include <QWidget>

class CircleProgress : public QWidget
{
    Q_OBJECT

public:
    explicit CircleProgress(QWidget* parent = nullptr);
    
    void setProgress(double progress);
    void setUsedText(const QString &text);

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    double m_progress;
    QString m_userText;
};

#endif // CIRCLEPROGRESS_H
