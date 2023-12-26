#include "chart_ellipse.h"

void Chart_Ellipse::paintChart(QPainter & p)
{
    QPen tmp = p.pen();
    p.setPen(paintChartDrawPen);

    int sx = paintStart.rx(),sy = paintStart.ry(),ex = paintEnd.rx(),ey = paintEnd.ry();

    if(graphPath) delete graphPath;
    graphPath = new QPainterPath;

    graphPath->addEllipse(sx,sy,ex-sx,ey-sy);

    //graphPath->addEllipse(0,0,100,100);

    p.fillPath(*graphPath,paintChartFillPen);
    p.drawPath(*graphPath);

    p.setPen(tmp);
}
