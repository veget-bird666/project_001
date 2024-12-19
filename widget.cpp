#include "widget.h"
#include "ui_widget.h"
#include <QPainter> // 添加此行
#include <QTimer>
#include "gamemap.h"
#include <vector>
#include <QMouseEvent>
#include <utility>  // 含pair

// 地图格子
std::vector<std::vector<Space*>> spaces;

// 爆炸位置
std::vector<std::vector<bool>> toBomb;

// 画板距离地图边界的距离
const int i_off = 80;
const int j_off = 50;

//方块大小
const int block_size = 50;

// 地图大小
const int row = 8;
const int col = 8;

// 重绘时间间隔
const int duration = 30;

// 选中方块坐标
std::vector<std::pair<int, int>> selected_points;

// 分数
int score = 0;

// 有关地图相关操作的函数
void mapInitial();
void SwapStone(int i1, int j1, int i2, int j2);
void performBomb();
void goDown();
void flush();
bool checkMap();
bool checkOne(int i, int j);
void clearMap();



Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , timer(new QTimer(this))
{

    // 加载图片
    picInitial();
    ui->setupUi(this);
    mapInitial();
    clearMap();

    // 连接 QTimer 的 timeout 信号到槽函数
    connect(timer, &QTimer::timeout, this, &Widget::updateWidget);

    // 启动定时器，每隔一段时间重绘一次
    timer->start(duration);

}

Widget::~Widget()
{
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < col; ++j) {
            delete spaces[i][j];
        }
    }

    delete ui;
}


// 实现paintEvent方法
void Widget::paintEvent(QPaintEvent *event)
{


    QPainter painter(this);
    for(int i=0;i<row;i++){
        for(int j=0;j<col;j++){
            int type = spaces[i][j]->GetGemstone()->GetType();
            painter.drawPixmap(j_off+j*block_size,i_off+i*block_size,pic_mines[type]);
        }
    }

    ui->scoreLabel->setText(QString::number(score));




    if(!selected_points.empty()){

        // 创建颜色对象，设置透明度
        QColor color(100, 100, 100, 150); // 150：透明度为约 59%
        int i = selected_points[0].first;
        int j = selected_points[0].second;
        ui->label_2->setText(QString::number(i)+","+QString::number(j));
        painter.fillRect(j_off + j*block_size,i_off + i*block_size,block_size,block_size,color);

    }
}


// 实现mousePressEvent方法
void Widget::mousePressEvent(QMouseEvent *event)
{
    int click_x = event->pos().x(); // 获取 x 坐标
    int click_y = event->pos().y(); // 获取 y 坐标

    ui->label->setText(QString::number(click_x)+","+QString::number(click_y));

    int i = (click_y-i_off)/block_size; // 获取当前i
    int j = (click_x-j_off)/block_size; // 获取当前j

    // 判断两次选择是否重复
    bool is_repeat = !selected_points.empty() && i == selected_points[0].first
                     && j == selected_points[0].second;

    if(i>=0 && i<row && j>=0 && j<col && !is_repeat){   // 判断i,j是否合法及重复
        selected_points.push_back(std::make_pair(i,j));



        // 若已经选择两个方块
        if(selected_points.size() == 2){
            int i1 = selected_points[0].first;
            int j1 = selected_points[0].second;
            int i2 = selected_points[1].first;
            int j2 = selected_points[1].second;
            SwapStone(i1,j1,i2,j2);
            selected_points.clear();
        }

    }






}

void Widget::updateWidget(){
    update(); // 请求重绘窗口
}



// 图片加载
void Widget::picInitial(){

    // 宝石图片的加载
    pic_mine1.load(":/material/picture/mine/mine1.png");
    pic_mine2.load(":/material/picture/mine/mine2.png");
    pic_mine3.load(":/material/picture/mine/mine3.png");
    pic_mine4.load(":/material/picture/mine/mine4.png");
    pic_mine5.load(":/material/picture/mine/mine5.png");
    pic_mine6.load(":/material/picture/mine/mine6.png");
    pic_mine7.load(":/material/picture/mine/mine7.png");
    pic_mine8.load(":/material/picture/mine/mine8.png");

    pic_mines = {pic_mine1,pic_mine2,pic_mine3,pic_mine4,pic_mine5,pic_mine6,pic_mine7,pic_mine8};

}





void mapInitial() {
    toBomb.assign(row, std::vector<bool>(col, false));
    spaces.resize(row, std::vector<Space*>(col));

    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            spaces[i][j] = new Space();
            Gemstone* gemstone = new Gemstone();
            spaces[i][j]->SetGemstone(gemstone);
        }
    }
}


// 交换格子的宝石
void SwapStone(int i1, int j1, int i2, int j2) {
    // 检查边界和邻接情况
    if (i1 < 0 || i1 >= row || j1 < 0 || j1 >= col ||
        i2 < 0 || i2 >= row || j2 < 0 || j2 >= col)
        return;

    // 两个格子必须相邻
    if (std::abs(i1 - i2) + std::abs(j1 - j2) != 1)
        return;

    std::cout << "进行交换" << std::endl;
    Gemstone* g1 = spaces[i1][j1]->GetGemstone();
    Gemstone* g2 = spaces[i2][j2]->GetGemstone();
    spaces[i1][j1]->SetGemstone(g2);
    spaces[i2][j2]->SetGemstone(g1);

    if (checkOne(i1, j1) || checkOne(i2, j2)) {
        std::cout << "交换后存在可消除部分" << std::endl;
        performBomb();
        goDown();
        flush();
        clearMap();
    } else {
        std::cout << "交换后不存在可消除部分" << std::endl;
        spaces[i1][j1]->SetGemstone(g1);
        spaces[i2][j2]->SetGemstone(g2);
    }
}

// 执行爆炸
void performBomb() {
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if (toBomb[i][j]) {
                delete spaces[i][j]->GetGemstone();
                spaces[i][j]->SetGemstone(nullptr);
                toBomb[i][j] = false;
                score += 50;

            }
        }
    }
}

// 实现下落
void goDown() {
    for (int i = row - 1; i >= 0; i--) {
        for (int j = col - 1; j >= 0; j--) {
            if (spaces[i][j]->GetGemstone() == nullptr) {
                int temp_i = i - 1;
                while (temp_i >= 0) {
                    if (spaces[temp_i][j]->GetGemstone() != nullptr) {
                        Gemstone* g = spaces[temp_i][j]->GetGemstone();
                        spaces[i][j]->SetGemstone(g);
                        spaces[temp_i][j]->SetGemstone(nullptr);
                        break;
                    }
                    temp_i--;
                }
            }
        }
    }
}

// 刷新空位置宝石
void flush() {
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if (spaces[i][j]->GetGemstone() == nullptr) {
                Gemstone* g = new Gemstone();
                spaces[i][j]->SetGemstone(g);
            }
        }
    }
}



// 判断地图是否有可消除部分
bool checkMap() {
    bool couldBomb = false;
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if (spaces[i][j]->GetGemstone() == nullptr) continue;

            if (j < col - 2 &&
                spaces[i][j]->GetGemstone()->GetType() == spaces[i][j + 1]->GetGemstone()->GetType() &&
                spaces[i][j + 1]->GetGemstone()->GetType() == spaces[i][j + 2]->GetGemstone()->GetType()) {

                couldBomb = true;
                toBomb[i][j] = true;
                toBomb[i][j + 1] = true;
                toBomb[i][j + 2] = true;
            }

            if (i < row - 2 &&
                spaces[i][j]->GetGemstone()->GetType() == spaces[i + 1][j]->GetGemstone()->GetType() &&
                spaces[i + 1][j]->GetGemstone()->GetType() == spaces[i + 2][j]->GetGemstone()->GetType()) {

                couldBomb = true;
                toBomb[i][j] = true;
                toBomb[i + 1][j] = true;
                toBomb[i + 2][j] = true;
            }
        }
    }
    return couldBomb;
}

// 检查一点为中心的5*5部分是否可消除
bool checkOne(int i, int j) {
    if (spaces[i][j]->GetGemstone() == nullptr) return false;

    bool couldBomb = false;
    int TargetType = spaces[i][j]->GetGemstone()->GetType();
    for (int di = -2; di <= 2; di++) {
        for (int dj = -2; dj <= 2; dj++) {
            int newI = i + di;
            int newJ = j + dj;

            if (newI >= 0 && newI < row && newJ >= 0 && newJ < col) {
                if (dj <= 0 && newJ + 2 < col &&
                    spaces[newI][newJ]->GetGemstone()->GetType() == TargetType &&
                    spaces[newI][newJ + 1]->GetGemstone()->GetType() == TargetType &&
                    spaces[newI][newJ + 2]->GetGemstone()->GetType() == TargetType) {

                    couldBomb = true;
                    toBomb[newI][newJ] = true;
                    toBomb[newI][newJ + 1] = true;
                    toBomb[newI][newJ + 2] = true;
                }

                if (di <= 0 && newI + 2 < row &&
                    spaces[newI][newJ]->GetGemstone()->GetType() == TargetType &&
                    spaces[newI + 1][newJ]->GetGemstone()->GetType() == TargetType &&
                    spaces[newI + 2][newJ]->GetGemstone()->GetType() == TargetType) {

                    couldBomb = true;
                    toBomb[newI][newJ] = true;
                    toBomb[newI + 1][newJ] = true;
                    toBomb[newI + 2][newJ] = true;
                }
            }
        }
    }
    return couldBomb;
}


// 清除地图至不可消除
void clearMap() {

    while(checkMap()){
        performBomb();
        goDown();
        flush();
    }

}
