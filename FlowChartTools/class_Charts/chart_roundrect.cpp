#include "chart_roundrect.h"

void Chart_RoundRect::paintChart(QPainter & p)
{
    QPen tmp = p.pen();
    p.setPen(paintChartDrawPen);

    int sx = paintStart.rx(),sy = paintStart.ry(),ex = paintEnd.rx(),ey = paintEnd.ry();

    if(graphPath) delete graphPath;
    graphPath = new QPainterPath;
    //graphPath->addRect(sx,sy,ex-sx,ey-sy-30);
    graphPath->addRoundedRect(sx,sy,ex-sx,ey-sy-30,20,20);

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

void Chart_RoundRect::buttonClicked()
{
    if(btn_open)
    {
        qDebug() << "Chart_RoundRect::btn_Open存在!!!" << endl;
        qDeleteAll(this->findChildren<QPushButton *>());
    }else
    {
        qDebug() << "Chart_RoundRect::btn_Open不存在!!!" << endl;
        //btn_open = new QPushButton("+",this);
    }


    qDebug() << "Chart_RoundRect::buttonClicked()::按钮被点击了!!!" << endl;
}

