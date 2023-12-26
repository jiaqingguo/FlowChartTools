#ifndef FLOWCHART_H
#define FLOWCHART_H
#pragma execution_character_set("utf-8")
#include <map>
#include <stdexcept>

#include <QWidget>
#include <QPainter>
#include <vector>
#include <QDebug>
#include <QMouseEvent>
#include <QPolygon>
#include<QStyleOption>
#include<QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QColorDialog>
#include <QButtonGroup>

#include <QMap>
#include <QPoint>

#include "class_Charts/chart_base.h"
#include "class_Charts/chart_rect.h"
#include "class_Charts/chart_diamond.h"
#include "class_Charts/chart_roundrect.h"
#include "class_Charts/chart_ellipse.h"
#include "class_Charts/chart_line.h"
#include "class_Charts/chart_trapezoid.h"
#include "class_Charts/chart_pentagram.h"
#include "class_Charts/chart_rectmy.h"
#include "class_Charts/chart_block.h"
#include "selectimgform.h"

#include "comdef.h"
struct BlockInfo
{
    QString name;//Block的名字
    QString refid;//block的ID persentation里面的refid
    int top_left_x;//左上角x坐标
    int top_left_y;//左上角y坐标
    int width;//宽
    int height;//高

    QVector<QPoint> MagneticPoint;//磁力点相对坐标位置
    QVector<QPoint> MagneticPoint_Abs;//磁力点绝对坐标的位置
    int SerialNumber;//创建序号 第几个创建的块
};
struct LineInfo
{
    QPoint AbsltCoordinates;//第一个坐标 这个是绝对位置 之后其他的都是相对于这个坐标的
    QPoint headCoordinates;//连线在头结点的坐标
    QPoint tailCoordinates;//连线在尾节点的坐标
    QString headRefid;//头结点的refid
    QString tailRefid;//尾结点的refid

};

class FlowChart : public QWidget
{
    Q_OBJECT
private:
    friend class Chart_Base;

    QVector<BlockInfo> m_vecBlock;//存放block信息
    QVector<LineInfo> m_vecline;//存放线的相关信息

    QMap<QString,QString> nodeNameAndID;
    QPushButton *btn2;                                          //跳出选择图片界面的按钮
    Chart_Base *curPaintChart = nullptr;                        // 将要放置的图形

    Chart_Base *testPaintChart1 = nullptr;                        // 将要放置的图形
    Chart_Base *testPaintChart2 = nullptr;                        // 将要放置的图形

    PaintChartType curPaintChartType = PaintChartType::NONE;    // 绘制的图形的类型

    int m_BtnID;  //存放为了显示图片新建的按钮的ID
    //int m_BtnID;  //存放为了显示图片新建的按钮的ID
    QButtonGroup *m_btnGroup;//存放新建按钮的组
    QPushButton *btn_Select = nullptr;//当前被选中的按钮


    Chart_Base *curSelecChart = nullptr;    // 选中的画板上的图形
    QPoint curSelecChartPos;                // 选中时指针在图像对象上的位置信息
    QPoint curSelecChartStart;                //选中时图形的左上角位置
    QPoint curSelecChartEnd;                //选中时图形的右下角位置

    int m_startStyle;//开始方向的箭头类型
    int m_endStyle;//结束方向的箭头类型

    Chart_Line *newLineChart = nullptr;             // 要创建的磁力点的连线的指针
    Chart_Base *newLineToSelectChart = nullptr;     // 要连接到的磁力点的图形的指针
    Chart_Base *newLineFromSelectChart = nullptr;   // 要创建的磁力点的图形的指针
    ORIENTION magPointDirect = ORIENTION::NONE;     // 选中时要创建的起始磁力点的方向类型
    int magPointFromIndex;                          // 要创建的磁力点的索引
    int magPointToIndex;                            // 要创建的磁力点的索引

    Chart_Base *lineSelectChart = nullptr;  // 运行时线的吸附到的图形的指针
    int magPointIndex;                      // 运行时线吸附到的图形的磁力点的索引

    ORIENTION sizePointDirect = ORIENTION::NONE;    // 选中时要改变大小的大小点的方向类型

    std::vector<Chart_Base*> charts;    // 画板上图形的集合
    std::vector<Chart_Base*> line;      // 画板上线的集合

    std::vector<Chart_Base*> chart_xml;    //解析xml出来的节点的集合
    MOUSE_EVENT_TYPE mouseEventType = MOUSE_EVENT_TYPE::NONE;   // 当前鼠标事件类型

    bool fileIsOpened = false;      // 是否打开了文件
    bool fileIsSaved = true;        // 是否保存了文件
    QString filePath;               // 打开文件的路径




    SelectImgForm *m_SelectImgForm = nullptr;//选择图片界面、
    QLabel * lable_img;
    QPushButton * Pb_generatedPb;


    void initVar();             // 变量默认初始化函数
    void resetFlowChartPanel(); // 重置流图画板
    void setFileSetSaved(bool isSaved);     // 设置文件未保存
    void setFileNameShow(bool isSaved);     // 设置窗体文件名
    void clearChartsLine(){                 // 释放图形、连线内存
        for(auto it = charts.begin();it!=charts.end();it++) {
            if(*it) delete *it;
            *it = nullptr;
        }
        for(auto it = line.begin();it!=line.end();it++) {
            if(*it) delete *it;
            *it = nullptr;
        }
    }

public:
    explicit FlowChart(QWidget *parent = nullptr , Qt::WindowFlags f = 0);
    virtual ~FlowChart()
    {
        //delete[] curPaintChart;
        //delete[] curSelecChart;
        //delete[] newLineChart;
        //delete[] newLineToSelectChart;
        newChartFile();
        clearChartsLine();

    }

    QVector<int> getThePoint(QVector<QVector<int> > vecVec,int topLeftx,int topLefty,int width,int height);//获取图形上的坐标

    Chart_Base * getCurPaintChart();    // 获取新的图形
    Chart_Base * getCurSeleChart();     // 获取当前选中的图形
    void addChart(Chart_Base *cb);        // 添加图形到图形容器
    bool delChart(Chart_Base *&cb);      // 删除图形
    void addLine(Chart_Base *cb);       // 添加线条到线条容器
    bool delLine(Chart_Base *&cb);       // 添加图形
    void hideMagSizeAll();              // 隐藏所有元素
    bool saveFile(QString filePath);    // 保存文件
    bool loadFile(QString filePath);    // 读取文件
    bool loadFile_my(QString filePath);


protected:
virtual void paintEvent(QPaintEvent *event);
virtual void mousePressEvent(QMouseEvent *event);
virtual void mouseMoveEvent(QMouseEvent *event);
virtual void mouseReleaseEvent(QMouseEvent *event);
virtual void keyPressEvent(QKeyEvent *ev);
virtual void keyReleaseEvent(QKeyEvent *ev);

//bool eventFilter(QObject *target, QEvent *event);
signals:
    void sendChartStyle(QPen &qp, QBrush &qb);                                                          // 发送图形颜色信息
    void sendLineStyle(QPen &qp, LINE_HEAD_TYPE &startLineHeadType, LINE_HEAD_TYPE &endLineHeadType);   // 发送线条颜色和线头类型信息
    void disableStyle();                        // 关闭主窗体颜色信息设置
    void updateWindowTitle(const QString &);    // 更新主窗体标题

    void signalTest();
public slots:
    void setPen(QPen &qp){curSelecChart->paintChartDrawPen = qp;}                                                       // 设置笔刷
    void setBrush(QBrush &qb){curSelecChart->paintChartFillPen = qb;}                                                   // 设置填充颜色
    void setLineWidth(int i){curSelecChart->paintChartDrawPen.setWidth(i);}                                             // 设置线宽
    void setLineStartStyle(int i){dynamic_cast<Chart_Line *>(curSelecChart)->setStartLineHeadType(LINE_HEAD_TYPE(i));}  // 设置起点线头类型
    void setLineEndStyle(int i){dynamic_cast<Chart_Line *>(curSelecChart)->setEndLineHeadType(LINE_HEAD_TYPE(i));}      // 设置终点线头类型
public:
    void setSelChartLineColor(const QColor &color);     // 设置图形线条颜色
    void setSelChartFillColor(const QColor &color);     // 设置图形填充颜色
    void createNode(QString name,int x,int y,int wide,int height,QVector<QPoint> vecPoint);

public slots:
    void setMousePressedFlag(MOUSE_EVENT_TYPE);         // 设置鼠标按下事件类型
    void setPaintChart();                           // 设置将要绘制的图形
    void setSelecChart(Chart_Base *, int ,int);     // 设置选中的画板的图像
//    void resetSeletcChart();

public slots:
    void setPaintChart_Rect(){curPaintChartType = PaintChartType::RECT;setPaintChart();}                // 获取一个新的矩形
    void setPaintChart_Diamond(){curPaintChartType = PaintChartType::DIAMOND;setPaintChart();}          // 获取一个新的菱形
    void setPaintChart_RoundRect(){curPaintChartType = PaintChartType::ROUNDRECT;setPaintChart();}      // 获取一个新的圆角矩形
    void setPaintChart_Ellipse(){curPaintChartType = PaintChartType::ELLIPSE;setPaintChart();}          // 获取一个新的圆形
    void setPaintChart_Line(){curPaintChartType = PaintChartType::LINE;setPaintChart();}                // 获取一个新的线段
    void setPaintChart_trapezoid(){curPaintChartType = PaintChartType::TRAPEZOID;setPaintChart();}      // 获取一个新的梯形
    void setPaintChart_pentagram(){curPaintChartType = PaintChartType::PENTAGRAM;setPaintChart();}      // 获得一个新的五角星形
    void resetPaintChartType(){curPaintChartType = PaintChartType::NONE;}                               // 清空类型
    void deleteChart_base();//删除选中的控件
    void recvModifyColor_Red();//修改颜色为红色
    void recvModifyColor_Green();//修改颜色为绿色
    void recvModifyColor_Blue();//修改颜色为蓝色
    void recvModifyColor_White();//修改颜色为白色

    void MyTest();
    void createBlock();
    void createLine_Combination();//新建表示组合的线
    void createLine_Quote();//新建表示引用的线
    void createLine_Generalization();//创建表示泛化的线
    void slot_RecvImagePath(QString path);
    void slot_clickedGroup(int num);//得到点击按钮的ID

    void loadExternalFiles();//加载外部文件 为了还原bdd图
    void createLine();//自定义创建线
    void createLine(Chart_Base *Chart1,QPoint chart1_p,int index1,Chart_Base *Chart2,QPoint chart2_p,int index2);//自定义创建线

    void createLine(int head_x,int head_y,int tail_x,int tail_y);//自定义创建线
    void createLine(QPoint head,QPoint tail);//自定义创建线
public slots:
    void setTypeChangeSize(ORIENTION i){mouseEventType = MOUSE_EVENT_TYPE::RUNTIME_CHANGE_SIZE;sizePointDirect = i; }                                                                                                       // 设置鼠标事件类型为改变大小
    void setTypeCreateMagPoint(Chart_Base *cb,ORIENTION d,int i){newLineFromSelectChart = cb;mouseEventType = MOUSE_EVENT_TYPE::RUNTIME_CREATE_MAGPOINT;magPointDirect = d;magPointFromIndex = i;newLineChart = nullptr; }  // 设置鼠标事件类型为创建线段

    bool openChartFile();   // 打开文件
    bool openChartFile_my();   // 打开文件
    bool saveChartFile();   // 保存文件
    bool newChartFile();    // 新建文件

};

#endif // FLOWCHART_H


