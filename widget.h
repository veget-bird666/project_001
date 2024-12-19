#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <vector>
QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    int x = 0;
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;

    QTimer *timer;  // 定义 QTimer 为类的成员变量

    // 图片成员变量
    QPixmap pic_mine1;
    QPixmap pic_mine2;
    QPixmap pic_mine3;
    QPixmap pic_mine4;
    QPixmap pic_mine5;
    QPixmap pic_mine6;
    QPixmap pic_mine7;
    QPixmap pic_mine8;

    // 图片数组
    std::vector<QPixmap> pic_mines;

    void picInitial();

    // 绘画函数
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    // 实现重绘
    void updateWidget();


};
#endif // WIDGET_H
