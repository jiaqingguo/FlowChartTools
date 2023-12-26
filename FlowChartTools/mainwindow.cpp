#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString tmp = "sysml语言构建器";
    this->setWindowTitle(tmp);

    connect(ui->chart_rect,SIGNAL(clicked()),ui->flowChart_widget,SLOT(setPaintChart_Rect()));
    connect(ui->chart_diam,SIGNAL(clicked()),ui->flowChart_widget,SLOT(setPaintChart_Diamond()));
    connect(ui->chart_roundrect,SIGNAL(clicked()),ui->flowChart_widget,SLOT(setPaintChart_RoundRect()));
    connect(ui->chart_ellipse,SIGNAL(clicked()),ui->flowChart_widget,SLOT(setPaintChart_Ellipse()));
    connect(ui->chart_line,SIGNAL(clicked()),ui->flowChart_widget,SLOT(setPaintChart_Line()));
    connect(ui->chart_trapezoid,SIGNAL(clicked()),ui->flowChart_widget,SLOT(setPaintChart_trapezoid()));
    connect(ui->chart_rectMy,SIGNAL(clicked()),ui->flowChart_widget,SLOT(MyTest()));
    connect(ui->chart_pentagram,SIGNAL(clicked()),ui->flowChart_widget,SLOT(setPaintChart_pentagram()));//点击五角星形的按钮
    //connect(ui->pb_deleteChartBase,SIGNAL(clicked()),ui->flowChart_widget,SLOT(deleteChart_base()));//按钮删除
    connect(ui->chart_block,SIGNAL(clicked()),ui->flowChart_widget,SLOT(createBlock()));//chart_block 创建<<block>>的按钮
//    connect(ui->pb_createLine,SIGNAL(clicked()),ui->flowChart_widget,SLOT(createLine()));
    connect(ui->chart_CombinationLine,SIGNAL(clicked()),ui->flowChart_widget,SLOT(createLine_Combination()));//创建组合线
    connect(ui->chart_QuoteLine,SIGNAL(clicked()),ui->flowChart_widget,SLOT(createLine_Quote()));//创建引用线
    connect(ui->chart_Generalization,SIGNAL(clicked()),ui->flowChart_widget,SLOT(createLine_Generalization()));//创建泛化线


    connect(ui->actionOpenFile,SIGNAL(triggered(bool)),ui->flowChart_widget,SLOT(openChartFile_my()));
    connect(ui->actionSaveFile,SIGNAL(triggered(bool)),ui->flowChart_widget,SLOT(saveChartFile()));
    connect(ui->actionNewFile,SIGNAL(triggered(bool)),ui->flowChart_widget,SLOT(newChartFile()));

    connect(ui->LineColorButton,SIGNAL(clicked(bool)),this,SLOT(sendSelChartLineColor()));
    connect(ui->FillColorButton,SIGNAL(clicked(bool)),this,SLOT(sendSelChartFillColor()));
    connect(ui->LineWidthCBox,SIGNAL(currentIndexChanged(int)),ui->flowChart_widget,SLOT(setLineWidth(int)));
    connect(ui->LineStartStyleCBox,SIGNAL(currentIndexChanged(int)),ui->flowChart_widget,SLOT(setLineStartStyle(int)));
    connect(ui->LineEndStyleCBox,SIGNAL(currentIndexChanged(int)),ui->flowChart_widget,SLOT(setLineEndStyle(int)));



    connect(ui->flowChart_widget,SIGNAL(sendChartStyle(QPen &, QBrush &)),this,SLOT(setFillStyle(QPen &, QBrush &)));
    connect(ui->flowChart_widget,SIGNAL(sendLineStyle(QPen &, LINE_HEAD_TYPE &, LINE_HEAD_TYPE &)),this,SLOT(setLineStyle(QPen &, LINE_HEAD_TYPE &, LINE_HEAD_TYPE &)));
    connect(ui->flowChart_widget,SIGNAL(disableStyle()),this,SLOT(disableStylePanel()));
    connect(ui->flowChart_widget,SIGNAL(updateWindowTitle(const QString &)),this,SLOT(changeWindowTitle(const QString &)));




    //connect(ui->actionCloseFile,SIGNAL(triggered(bool)),ui->flowChart_widget,SLOT(saveChartFile()));

    //ui->abcdef->setGeometryNew(10,10,200,200);
    //ui->abcdef->update();
    //ui->flowChart_widget->setStyleSheet(QStringLiteral("background-color: rgb(206, 232, 255);"));
    //ui->flowChart_widget->setAttribute(Qt::WA_StyledBackground,true);
    //ui->flowChart_widget->update();
}

MainWindow::~MainWindow()
{
    ui->flowChart_widget->newChartFile();
    delete ui;
}

void MainWindow::setFillStyle(QPen &qp, QBrush &qb)
{
    ui->FillColorGroup->setEnabled(true);
    ui->LineColorGroup->setEnabled(true);
    ui->FillColorButton->setEnabled(true);
    ui->LineColorButton->setEnabled(true);
    ui->LineColorButton->setStyleSheet(QString("QPushButton\
    {\
        background-color:rgb(%1,%2,%3);\
        opacity:1;\
    };"
    ).arg(qp.color().red()).arg(qp.color().green()).arg(qp.color().blue()));
    ui->FillColorButton->setStyleSheet(QString("QPushButton\
    {\
        background-color:rgb(%1,%2,%3);\
        opacity:1;\
    };"
    ).arg(qb.color().red()).arg(qb.color().green()).arg(qb.color().blue()));

}

void MainWindow::setLineStyle(QPen &qp,LINE_HEAD_TYPE &startLineHeadType, LINE_HEAD_TYPE &endLineHeadTypeb)
{
    ui->LineColorGroup->setEnabled(true);
    ui->LineWidthGroup->setEnabled(true);
    ui->LineStartStyleGroup->setEnabled(true);
    ui->LineEndStyleGroup->setEnabled(true);
    ui->LineColorButton->setEnabled(true);
    ui->LineWidthCBox->setEnabled(true);
    ui->LineStartStyleCBox->setEnabled(true);
    ui->LineEndStyleCBox->setEnabled(true);
    ui->LineColorButton->setStyleSheet(QString("QPlainTextEdit\
    {\
        background-color:rgb(%1,%2,%3);\
        opacity:1;\
    }"
    ).arg(qp.color().red()).arg(qp.color().green()).arg(qp.color().blue()));
    ui->LineWidthCBox->setCurrentIndex(qp.width());
    ui->LineStartStyleCBox->setCurrentIndex(int(startLineHeadType));
    ui->LineEndStyleCBox->setCurrentIndex(int(endLineHeadTypeb));
}

void MainWindow::disableStylePanel()
{
    ui->FillColorGroup->setEnabled(false);
    ui->LineColorGroup->setEnabled(false);
    ui->LineWidthGroup->setEnabled(false);
    ui->LineStartStyleGroup->setEnabled(false);
    ui->LineEndStyleGroup->setEnabled(false);
    ui->LineColorButton->setStyleSheet(QString("QPushButton\
    {\
        opacity:0.9;\
    };"));
    ui->FillColorButton->setStyleSheet(QString("QPushButton\
    {\
        opacity:0.9;\
    };"));
}

void MainWindow::sendSelChartLineColor()
{
    QColor color = QColorDialog::getColor(Qt::white,this,tr("设置线条颜色"));
    ui->flowChart_widget->setSelChartLineColor(color);
    ui->LineColorButton->setStyleSheet(QString("QPushButton\
    {\
        background-color:rgb(%1,%2,%3);\
        opacity:1;\
    };"
    ).arg(color.red()).arg(color.green()).arg(color.blue()));
}

void MainWindow::sendSelChartFillColor()
{
    QColor color = QColorDialog::getColor(Qt::white,this,tr("设置填充颜色"));
    ui->flowChart_widget->setSelChartFillColor(color);
    ui->FillColorButton->setStyleSheet(QString("QPushButton\
    {\
        background-color:rgb(%1,%2,%3);\
        opacity:1;\
    };"
    ).arg(color.red()).arg(color.green()).arg(color.blue()));
}
void MainWindow::changeWindowTitle(const QString & title)
{
    QString tmp = "sysml语言构建器";
    this->setWindowTitle(tmp);
}




void MainWindow::on_flowChart_widget_customContextMenuRequested(const QPoint &pos)
{
    //创建菜单对象
    QMenu *pMenu = new QMenu(this);

    QAction *pDleteChart = new QAction(tr("删除"), this);
    QAction *pModifyColor = new QAction(tr("修改颜色"),this);

    //1:新建任务 2:设置任务 3:删除任务 4:改名工具 5:设置工具 6:删除工具
    pDleteChart->setData(1);
    pModifyColor->setData(2);

    //把QAction对象添加到菜单上
    pMenu->addAction(pDleteChart);
    pMenu->addAction(pModifyColor);
//    pMenu->addAction(pEditTask);

    //子菜单
    QMenu *childMenu_mdfColor = new QMenu();
    //子菜单的 子项
    QAction *mdf_red = new QAction(childMenu_mdfColor);
    mdf_red->setText("红色");
    QAction *mdf_green = new QAction(childMenu_mdfColor);
    mdf_green->setText("绿色");
    QAction *mdf_blue = new QAction(childMenu_mdfColor);
    mdf_blue->setText("蓝色");
    QAction *mdf_white = new QAction(childMenu_mdfColor);
    mdf_white->setText("白色");
    QList<QAction *> childActionList_color;
    childActionList_color<<mdf_red\
                   <<mdf_green\
                 <<mdf_blue\
                <<mdf_white;
    childMenu_mdfColor->addActions(childActionList_color);
    //设置子菜单 归属opion
    pModifyColor->setMenu(childMenu_mdfColor);
    //主菜单添加子菜单
    pMenu->addMenu(childMenu_mdfColor);

    //连接鼠标右键点击信号
    connect(pDleteChart, SIGNAL(triggered()), ui->flowChart_widget, SLOT(deleteChart_base()));
    connect(mdf_red,SIGNAL(triggered()),ui->flowChart_widget,SLOT(recvModifyColor_Red()));
    connect(mdf_green,SIGNAL(triggered()),ui->flowChart_widget,SLOT(recvModifyColor_Green()));
    connect(mdf_blue,SIGNAL(triggered()),ui->flowChart_widget,SLOT(recvModifyColor_Blue()));
    connect(mdf_white,SIGNAL(triggered()),ui->flowChart_widget,SLOT(recvModifyColor_White()));

    //在鼠标右键点击的地方显示菜单
    pMenu->exec(cursor().pos());

    //释放内存
    QList<QAction*> list = pMenu->actions();
    foreach (QAction* pAction, list) delete pAction;
    delete pMenu;
}




