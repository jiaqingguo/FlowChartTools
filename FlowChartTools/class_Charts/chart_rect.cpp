#include "chart_rect.h"
#include <QDebug>
//#include <QFont>
void Chart_Rect::paintChart(QPainter & p)
{
//    qDebug() << "Chart_Rect::paintChart" << endl;
    QPen tmp = p.pen();
    p.setPen(paintChartDrawPen);

    int sx = paintStart.rx(),sy = paintStart.ry(),ex = paintEnd.rx(),ey = paintEnd.ry();

    if(graphPath) delete graphPath;
    graphPath = new QPainterPath;
    //graphPath->addRect(sx,sy,ex-sx,ey-sy-30);
    graphPath->addRect(sx,sy,ex-sx,ey-sy);


//    QFont ft;
//    ft.setPointSize(5);
//    ft.setBold(true);
//        label_Show = new QLabel("<<block>>",this);
//        label_Show->setFont(ft);
//        label_Show->setStyleSheet("QLabel{color:red;background:white}");
//        label_Show->move((ex-sx)/2,ey-30);
//        label_Show->show();
//        label_Show->resize(30,80);

    //反走样
    p.setRenderHint(QPainter::Antialiasing, true);

    // 绘制图标
   // p.drawPixmap(rect(), QPixmap(":/img/2000c1000.jpg"));//设置背景图片


//    QFont ft;
//    ft.setPointSize(20);
//    ft.setBold(true);
//    btn_open = new QPushButton("+",this);
//    btn_open->setFont(ft);
//    btn_open->setStyleSheet("QPushButton{color:red;background:white}");
//    btn_open->move(ex-(ex-sx)/2-15,ey-30);
//    btn_open->show();
//    btn_open->resize(30,30);

    p.fillPath(*graphPath,paintChartFillPen);
    p.drawPath(*graphPath);

    p.setPen(tmp);
}

void Chart_Rect::updateMagPointInfo()
{
    int x1 = paintStart.rx(),y1 = paintStart.ry();
    int x2 = paintEnd.rx(),y2 = paintEnd.ry();
    int midx = ((paintStart.rx() + paintEnd.rx())>>1);
    int midy = ((paintStart.ry() + paintEnd.ry())>>1);
    int midTest = midy/2;

    magPoint.i_point[0]->setX(x1);
    magPoint.i_point[0]->setY(y1);
    magPoint.i_point[0]->setRotate(ORIENTION::NORTHWEST);
    magPoint.i_point[1]->setX(midx);
    magPoint.i_point[1]->setY(y1);
    magPoint.i_point[1]->setRotate(ORIENTION::NORTH);
    magPoint.i_point[2]->setX(x2);
    magPoint.i_point[2]->setY(y1);
    magPoint.i_point[2]->setRotate(ORIENTION::NORTHEAST);
    magPoint.i_point[3]->setX(x2);
    magPoint.i_point[3]->setY(midy);
    magPoint.i_point[3]->setRotate(ORIENTION::EAST);
    magPoint.i_point[4]->setX(x2);
    magPoint.i_point[4]->setY(y2);
    magPoint.i_point[4]->setRotate(ORIENTION::SOUTHEAST);
    magPoint.i_point[5]->setX(midx);
    magPoint.i_point[5]->setY(y2);
    magPoint.i_point[5]->setRotate(ORIENTION::SOUTH);
    magPoint.i_point[6]->setX(x1);
    magPoint.i_point[6]->setY(y2);
    magPoint.i_point[6]->setRotate(ORIENTION::SOUTHWEST);
    magPoint.i_point[7]->setX(x1);
    magPoint.i_point[7]->setY(midy);
    magPoint.i_point[7]->setRotate(ORIENTION::WEST);
    magPoint.i_point[8]->setX(x1);
    magPoint.i_point[8]->setY(midTest);
    magPoint.i_point[8]->setRotate(ORIENTION::WEST);

}

void Chart_Rect::buttonClicked()
{
    if(btn_open)
    {
        qDebug() << "btn_Open存在!!!" << endl;
        qDeleteAll(this->findChildren<QPushButton *>());
    }else
    {
        qDebug() << "btn_Open不存在!!!" << endl;
        //btn_open = new QPushButton("+",this);
    }


    qDebug() << "Chart_Rect::buttonClicked()::按钮被点击了!!!" << endl;
}
