#include "flowchart.h"
#include <QColor>
#include <QMessageBox>
#include <QFont>
#include <QWidget>


#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QDebug>
#include <QTextStream>//文本流
//#include <QMessageBox>
#include <QDomDocument>

FlowChart::FlowChart(QWidget *parent, Qt::WindowFlags f) : QWidget(parent,f)
{
//    this->grabKeyboard();
//    installEventFilter(this);
    initVar();
    setMouseTracking(true);
    update();

//    qDebug() << "FlowChart::FlowChart(QWidget *parent, Qt::WindowFlags f) : QWidget(parent,f)" << endl;
    m_SelectImgForm = new SelectImgForm();

    m_startStyle = 0;//开始方向的箭头类型
    m_endStyle = 0;//结束方向的箭头类型
    connect(m_SelectImgForm,SIGNAL(signal_SendImgPath(QString)),this,SLOT(slot_RecvImagePath(QString)));
    m_btnGroup = new QButtonGroup(this);
    m_BtnID = 1;
    connect(m_btnGroup,SIGNAL(buttonClicked(int)),this,SLOT(slot_clickedGroup(int)));//组按钮事件

}

void FlowChart::initVar()
{
    mouseEventType = MOUSE_EVENT_TYPE::NONE;
    curPaintChart = nullptr;
    curSelecChart = nullptr;
    newLineChart = nullptr;
    newLineFromSelectChart = nullptr;
    newLineToSelectChart = nullptr;
}

QVector<int> FlowChart::getThePoint(QVector<QVector<int> > vecVec,int topLeftx,int topLefty,int width,int height)
{
    QVector<int> vec;
    for (int i = 0; i < vecVec.size(); i++)
    {
        for (int j = 0; j < vecVec[i].size(); j+=2)
        {
            //cout << vecVec[i][j];
            if(vecVec[i][j] >= topLeftx && vecVec[i][j] <= (topLeftx+width)
              && vecVec[i][j+1] >= topLefty && vecVec[i][j+1] <= (topLefty+height))
            {
                //return vec;
                vec.push_back(vecVec[i][j]);
                vec.push_back(vecVec[i][j+1]);
                return vec;
            }
        }
    }


}
void FlowChart::resetFlowChartPanel()
{
    Chart_Base::resetStaticVal();
    curPaintChart = nullptr;
    curPaintChartType = PaintChartType::NONE;
    curSelecChart = nullptr;
    curSelecChartPos = QPoint();
    newLineChart = nullptr;
    newLineToSelectChart = nullptr;
    newLineFromSelectChart = nullptr;
    magPointDirect = ORIENTION::NONE;
    magPointFromIndex = 0;
    magPointToIndex = 0;
    sizePointDirect = ORIENTION::NONE;
    clearChartsLine();
    charts.reserve(0);
    charts.resize(0);
    line.reserve(0);
    line.resize(0);
    mouseEventType = MOUSE_EVENT_TYPE::NONE;
    fileIsOpened = false;
    fileIsSaved = true;
    filePath = "";
    setFileNameShow(fileIsSaved);
}

void FlowChart::setFileSetSaved(bool isSaved)
{
    if(fileIsSaved != isSaved)
    {
        fileIsSaved = isSaved;
        setFileNameShow(fileIsSaved);
    }
}

void FlowChart::setFileNameShow(bool isSaved)
{
    QString tmp;
    if(fileIsOpened)
    {
        tmp = QString("%1%2").arg(filePath).arg((isSaved?"":" *"));
        emit updateWindowTitle(tmp);
    }else
    {
        tmp = QString("%1%2").arg("Untitle").arg((isSaved?"":" *"));
        emit updateWindowTitle(tmp);
    }
}

Chart_Base * FlowChart::getCurPaintChart()
{
    return curPaintChart;
}

void FlowChart::setSelChartLineColor(const QColor &color)
{
    curSelecChart->paintChartDrawPen.setColor(color);
    curSelecChart->update();
}

void FlowChart::setSelChartFillColor(const QColor &color)
{
    curSelecChart->paintChartFillPen.setColor(color);
    curSelecChart->update();
}

void FlowChart::setMousePressedFlag(MOUSE_EVENT_TYPE f)
{
    mouseEventType = f;
}

void FlowChart::setPaintChart()
{
    if(curPaintChart != nullptr)
        delete curPaintChart;

    switch(curPaintChartType)
    {
        case PaintChartType::RECT:
        {
            curPaintChart = new Chart_Rect(this);

        }break;
        case PaintChartType::DIAMOND:
        {
            curPaintChart = new Chart_Diamond(this);
        }break;
        case PaintChartType::ROUNDRECT:
        {
            curPaintChart = new Chart_RoundRect(this);
        }break;
        case PaintChartType::ELLIPSE:
        {
            curPaintChart = new Chart_Ellipse(this);
        }break;
        case PaintChartType::LINE:
        {
            curPaintChart = new Chart_Line(this);
        }break;
        case PaintChartType::TRAPEZOID:
        {
            curPaintChart = new Chart_Trapezoid(this);
        }break;
        case PaintChartType::PENTAGRAM:
        {
            curPaintChart = new Chart_Pentagram(this);
        }break;
        default:case PaintChartType::NONE:{
            curPaintChart = nullptr;
        }break;
    }
    //connect(this,SIGNAL(signalTest()),curPaintChart,SLOT(buttonClicked()));
    if(curPaintChart)
    {
        mouseEventType = MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE;

    }
}

void FlowChart::setSelecChart(Chart_Base * cb, int x, int y)
{

    emit disableStyle();
//    this->grabKeyboard();
    if(curSelecChart != nullptr) curSelecChart->hideMagSize();
    curSelecChart = cb;
    curSelecChart->showMagSize();
    curSelecChartPos = QPoint(x,y);
    mouseEventType = MOUSE_EVENT_TYPE::RUNTIME_CHANGE_POS;
    Chart_Line *cl = nullptr;
    if(nullptr == (cl = dynamic_cast<Chart_Line*>(cb)))
    {
        emit sendChartStyle(cb->paintChartDrawPen,cb->paintChartFillPen);
    }else
    {
        emit sendLineStyle(cl->paintChartDrawPen,cl->getStartLineHeadType(),cl->getEndLineHeadType());
    }
    curSelecChartStart = curSelecChart->GetWightStart();
//    qDebug() << "FlowChart::setSelecChart->GetWightStart()" << curSelecChartStart.rx() << endl ;
    curSelecChartEnd = curSelecChart->GetWightEnd();
//    qDebug() << "curSelecChart->GetWightEnd()" << curSelecChartEnd.rx() << endl ;
}

bool FlowChart::openChartFile()
{
    if(!fileIsSaved)
    {
        QMessageBox tmp(QMessageBox::Warning,tr("警告！"),tr("不保存文件就关闭？"),QMessageBox::NoButton,this->parentWidget());
        QPushButton *saveclose = tmp.addButton(tr("保存并关闭"),QMessageBox::ActionRole);
        QPushButton *nosaveclose = tmp.addButton(tr("不保存关闭"),QMessageBox::AcceptRole);
        QPushButton *cancel = tmp.addButton(tr("取消"),QMessageBox::RejectRole);
        tmp.exec();
        if(tmp.clickedButton() == saveclose)
        {
            if(saveChartFile())
            {
                resetFlowChartPanel();
            }else
                return false;
            //qDebug()<<"Save and Quit";
        }else if(tmp.clickedButton() == nosaveclose)
        {
            //qDebug()<<"Not Save and Quit";
            resetFlowChartPanel();
        }else if(tmp.clickedButton() == cancel)
        {
            return true;
            //qDebug()<<"cancel ";
        }
    }
    resetFlowChartPanel();
    QString tmpFilePath = QFileDialog::getOpenFileName(this,tr("打开文件"),"F:",tr("FCT文件(*.fct)"));
    if(tmpFilePath == "") return false;

    if(loadFile(tmpFilePath) == false)
    {
        QMessageBox::critical(this->parentWidget(),tr("错误！"),tr("打开文件失败！\n文件损坏或类型不正确"),QMessageBox::Ok);
        return false;
    }else
    {
        fileIsOpened = true;
        this->filePath = tmpFilePath;
        setFileNameShow(fileIsSaved);
        qDebug()<<"Load File Success"<<fileIsSaved;
    }
    qDebug()<<"charts size:"<<charts.size()<<" line size::"<<line.size();
    return true;
}

// 打开文件
bool FlowChart::openChartFile_my()
{
    QString tmpFilePath = QFileDialog::getOpenFileName(this,tr("打开文件"),"D:",tr("gapchor文件(*.gaphor)"));
    if(tmpFilePath == "") return false;

    if(loadFile_my(tmpFilePath) == false)
    {
        QMessageBox::critical(this->parentWidget(),tr("错误！"),tr("打开文件失败！\n文件损坏或类型不正确"),QMessageBox::Ok);
        return false;
    }else
    {
        fileIsOpened = true;
        this->filePath = tmpFilePath;
        setFileNameShow(fileIsSaved);
        qDebug() << filePath << endl;
        qDebug()<<"Load File Success"<<fileIsSaved;
    }

    return true;
}
bool FlowChart::saveChartFile()
{
    if(fileIsSaved)
    {
        return true;
    }else
    {
        if(fileIsOpened)
        {
            if(saveFile(filePath))
            {
                filePath = "";
            }else
            {
                QMessageBox::critical(this->parentWidget(),tr("错误！"),tr("保存文件失败！\n文件已占用或者访问权限不足"),QMessageBox::Ok);
                return false;
            }
        }else{
            QString tmpFilePath = QFileDialog::getSaveFileName(this,tr("保存文件"),"F:",tr("FCT文件(*.fct)"));
            if(tmpFilePath == "") return false;
            if(saveFile(tmpFilePath))
            {
                filePath = tmpFilePath;
                fileIsOpened = true;
            }else
            {
                QMessageBox::critical(this->parentWidget(),tr("错误！"),tr("保存文件失败！\n文件已占用或者访问权限不足"),QMessageBox::Ok);
                return false;
            }
        }
        setFileSetSaved(true);
    }
    return true;
}
bool FlowChart::newChartFile()
{
    if(!fileIsSaved)
    {
        QMessageBox tmp(QMessageBox::Warning,tr("警告！"),tr("不保存文件就关闭？"),QMessageBox::NoButton,this->parentWidget());
        QPushButton *saveclose = tmp.addButton(tr("保存并关闭"),QMessageBox::ActionRole);
        QPushButton *nosaveclose = tmp.addButton(tr("不保存关闭"),QMessageBox::AcceptRole);
        QPushButton *cancel = tmp.addButton(tr("取消"),QMessageBox::RejectRole);
        tmp.exec();
        if(tmp.clickedButton() == saveclose)
        {
            if(saveChartFile())
            {
                resetFlowChartPanel();
            }else{
                return false;
            }
            //qDebug()<<"Save and Quit";

        }else if(tmp.clickedButton() == nosaveclose)
        {
            //qDebug()<<"Not Save and Quit";
            resetFlowChartPanel();
        }else if(tmp.clickedButton() == cancel)
        {
            //qDebug()<<"cancel ";
        }
    }else{
        resetFlowChartPanel();
    }
    return true;
}

//void FlowChart::resetSeletcChart()
//{
//    if(curSelecChart != nullptr) curSelecChart->hideMagSize();
//    curSelecChart = nullptr;
//}

Chart_Base * FlowChart::getCurSeleChart()
{
    return curSelecChart;
}


void FlowChart::addChart(Chart_Base *cb)
{
    charts.push_back(cb);
}
bool FlowChart::delChart(Chart_Base *&cb)
{
    for(auto it = charts.begin();it != charts.end();it++)
    {
        if(*it == cb)
        {
            Chart_Base *tmp = cb;
            cb = nullptr;
            charts.erase(it);
#if 1
            for(auto magit = tmp->magPoint.i_point.begin();magit!=tmp->magPoint.i_point.end();magit++)
            {
                for(auto magLineStIt = (*magit)->i_lineStart.begin();magLineStIt != (*magit)->i_lineStart.end();magLineStIt++)
                {
                    dynamic_cast<Chart_Line*>(*magLineStIt) ->resetEndChart();
                    if(!delLine(*magLineStIt))
                    {
                        qDebug()<<"Error";
                    }else{
                        qDebug()<<"Succeed.";
                    }
                }
                for(auto magLineEnIt = (*magit)->i_lineEnd.begin();magLineEnIt != (*magit)->i_lineEnd.end();magLineEnIt++)
                {
                    dynamic_cast<Chart_Line*>(*magLineEnIt) ->resetStartChart();
                    if(!delLine(*magLineEnIt))
                    {
                        qDebug()<<"Error";
                    }else{
                        qDebug()<<"Succeed.";
                    }
                }
            }
#endif
            delete[] tmp;

            return true;
        }
    }
    return false;
}
void FlowChart::addLine(Chart_Base *cb)
{
    line.push_back(cb);
}
bool FlowChart::delLine(Chart_Base *&cb)
{
    for(auto it = line.begin();it != line.end();it++)
    {
        if(*it == cb)
        {
            Chart_Line *tmp = dynamic_cast<Chart_Line*>(*it);
            cb = nullptr;
            line.erase(it);
            qDebug()<<tmp->magPoint.i_point.size();


            tmp->resetEndChart();
            tmp->resetStartChart();
            delete tmp;
            return true;
        }
    }
    return false;
}

void FlowChart::hideMagSizeAll()
{
    for(auto it = charts.begin();it!=charts.end();it++)
    {
        (*it)->hideMagSize();
    }
    for(auto it = line.begin();it!=line.end();it++)
    {
        (*it)->hideMagSize();
    }
    curSelecChart = nullptr;
    mouseEventType = MOUSE_EVENT_TYPE::NONE;
    emit disableStyle();
}

bool FlowChart::saveFile(QString filePath)
{
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    QDataStream fout(&file);

    Chart_Base::saveStaticValue(fout);
    unsigned long long i;
    i = charts.size();
    fout.writeRawData(reinterpret_cast<const char*>(&i),sizeof(unsigned long long));
    for(auto it = charts.begin(); it!=charts.end(); it++)
    {
        fout<<*(*it);
    }
    i = line.size();
    fout.writeRawData(reinterpret_cast<const char*>(&i),sizeof(unsigned long long));
    for(auto it = line.begin(); it!=line.end(); it++)
    {
        fout<<*(*it)<<*(reinterpret_cast<const Chart_Line*>(*it));
    }
    file.close();
    return true;
}

bool FlowChart::loadFile(QString filePath)
{
    QFile file(filePath);
    file.open(QIODevice::ReadOnly);
    QDataStream fin(&file);
    std::map<int,Chart_Base*> chartMap;

    Chart_Base::loadStaticValue(fin);
    unsigned long long cnt;
    fin.readRawData(reinterpret_cast<char*>(&cnt),sizeof(unsigned long long));
    qDebug()<<"图形个数："<<cnt;
    for(unsigned long long i = 0;i<cnt;i++)
    {
        PaintChartType tmp;
        Chart_Base *cb;
        fin.readRawData(reinterpret_cast<char*>(&tmp),sizeof(PaintChartType));
        switch(tmp)
        {

            case PaintChartType::RECT:
            {
                cb = new Chart_Rect(this);
            }break;
            case PaintChartType::DIAMOND:
            {
                cb = new Chart_Diamond(this);
            }break;
            case PaintChartType::ROUNDRECT:
            {
                cb = new Chart_RoundRect(this);
            }break;
            case PaintChartType::ELLIPSE:
            {
                cb = new Chart_Ellipse(this);
            }break;
            case PaintChartType::TRAPEZOID:
            {
                cb = new Chart_Trapezoid(this);
            }break;
            case PaintChartType::PENTAGRAM:
            {
                cb = new Chart_Pentagram(this);
            }break;
            default:case PaintChartType::NONE:{
                cb = nullptr;
            }break;
        }
        cb->chartType = tmp;
        fin>>(*cb);
        connect(cb,SIGNAL(sendThisClass(Chart_Base *, int,int)),this,SLOT(setSelecChart(Chart_Base *, int,int)));
        connect(cb,SIGNAL(setTypeChangeSize(ORIENTION)),this,SLOT(setTypeChangeSize(ORIENTION)));
        connect(cb,SIGNAL(setTypeCreateMagPoint(Chart_Base *,ORIENTION,int)),this,SLOT(setTypeCreateMagPoint(Chart_Base *,ORIENTION,int)));
        addChart(cb);
        cb->applyWidthHeight();
        cb->update();
        cb->show();
        chartMap[cb->getID()] = cb;
    }
    fin.readRawData(reinterpret_cast<char*>(&cnt),sizeof(unsigned long long));
    qDebug()<<"连线个数："<<cnt;
    for(unsigned long long i = 0;i<cnt;i++)
    {
        PaintChartType tmp;
        Chart_Base *cb;
        Chart_Line *cl;
        int id;
        fin.readRawData(reinterpret_cast<char*>(&tmp),sizeof(PaintChartType));
        cb = new Chart_Line(this);
        addLine(cb);
        if(nullptr == (cl = dynamic_cast<Chart_Line*>(cb))) qDebug()<<"error";
        fin>>(*cb)>>(*cl);
        try{
            fin.readRawData(reinterpret_cast<char*>(&id),sizeof(int));
            if(id>=0)
            {
                Chart_Base *cbs = chartMap.at(id);
                cbs->addMagiPointStartLine(cl->getStartMagIndex(),cl);
                cl->setStartChart(cbs);
            }
        }catch(std::out_of_range &oor){
            qDebug()<<oor.what()<<"Not Found Start chart.";
        }
        try{
            fin.readRawData(reinterpret_cast<char*>(&id),sizeof(int));
            if(id>=0)
            {
                Chart_Base *cbe = chartMap.at(id);
                cbe->addMagiPointEndLine(cl->getEndMagIndex(),cl);
                cl->setEndChart(cbe);
            }
        }catch(std::out_of_range &oor){
            qDebug()<<oor.what()<<"Not Found End chart.";
        }
        connect(cl,SIGNAL(sendThisClass(Chart_Base *, int,int)),this,SLOT(setSelecChart(Chart_Base *, int,int)));
        connect(cl,SIGNAL(setTypeChangeSize(ORIENTION)),this,SLOT(setTypeChangeSize(ORIENTION)));
        cl->applyWidthHeight();
        cl->update();
        cl->show();
    }
    hideMagSizeAll();
    return true;
}

bool FlowChart::loadFile_my(QString filePath)
{
    qDebug() << "FlowChart::loadFile_my: filePath" << filePath << endl;
    QFile file(filePath);//tx
    if (!file.open(QFileDevice::ReadOnly))
    {
        qDebug() << "文件打开失败!!!" << endl;
    }

    QDomDocument doc;

    // 将doc与file关联起来
    // 这个函数从IO设备dev中读取XML文档，如果内容被成功解析，则返回true;否则返回false。
    if (!doc.setContent(&file))
    {
        qDebug() << "操作的不是xml文件!!!" << endl;
    }else
    {
        qDebug() << "是xml文件!!!" << endl;
    }

    QDomElement root = doc.documentElement();
    // 获取所有Book1节点
    QDomNodeList list1 = root.elementsByTagName("Block");//Block
    for (int i = 0; i < list1.count(); i++)
    {
        // 获取链表中的值
        QDomElement element1 = list1.at(i).toElement();
        //找到需要读取的节点 Block
        //qDebug() << "子节点：" << element1.nodeName();
        // 获得子节点
        QDomNode name = element1.namedItem("name");
        QDomNode nameVal = name.namedItem("val");
        QString nameString = nameVal.firstChild().nodeValue();
        /******************获取presentation里面的reflist里面的ref refid的值*********/
        QDomElement presentation = element1.firstChildElement("presentation");
        QDomElement reflist = presentation.firstChildElement("reflist");
        QDomElement ref = reflist.firstChildElement("ref");
        QString refid =  ref.attribute("refid");
        qDebug() << "refid:" << refid << endl;
        nodeNameAndID.insert(refid,nameString);

    }

    // 获取所有BlockItem节点
    QDomNodeList list_BlockItem = root.elementsByTagName("BlockItem");//Block
    for (int i = 0; i < list_BlockItem.count(); i++)
    {
        //m_vecBlock
        BlockInfo block;
        // 获取链表中的值
        QDomElement element_blockItem= list_BlockItem.at(i).toElement();
        //找到需要读取的节点 Block

        // 通过attribute方法返回属性中的值
        QString name = nodeNameAndID.value(element_blockItem.attribute("id"));

        qDebug() << "子节点：" << element_blockItem.nodeName() << "  Name:" << name ;
        // 获得子节点
        QDomNode matrix = element_blockItem.namedItem("matrix");
        QDomNode matrixVal = matrix.namedItem("val");
        QString matrixVal_str = matrixVal.firstChild().nodeValue();
        //qDebug() << "matrixVal:" << matrixVal_str <<endl;
        //t.remove(QChar('\n'), Qt::CaseInsensitive);
        matrixVal_str.remove(QChar(')'), Qt::CaseInsensitive);
        QStringList matrixVal_list = matrixVal_str.split(",");//QString字符串分割函数

        QString str_x = matrixVal_list[matrixVal_list.count()-2];
        QString str_y = matrixVal_list[matrixVal_list.count()-1];
        //qDebug() << "str_x:" << str_x << "str_y:" << str_y <<endl;
        /*********************获得子左上角坐标***********************/

        QDomNode top_left = element_blockItem.namedItem("top-left");
        QDomNode top_leftVal = top_left.namedItem("val");
        QString top_leftValstr = top_leftVal.firstChild().nodeValue();
        top_leftValstr.remove(QChar('('), Qt::CaseInsensitive);
        top_leftValstr.remove(QChar(')'), Qt::CaseInsensitive);
        QStringList top_leftValList = top_leftValstr.split(",");//QString字符串分割函数
        QString str_leftx = top_leftValList[0];
        QString str_lefty = top_leftValList[1];
        //qDebug() << "str_leftx:" << str_leftx << "str_lefty:" << str_lefty <<endl;
        double x = str_leftx.toDouble() + str_x.toDouble();//左上角坐标是top-left里val的x+matrix里val的x
        double y = str_lefty.toDouble() + str_y.toDouble();
        //qDebug() << "x:" << x << "y:" <<  y << endl;
        /*********************获取宽************************/
        QDomNode width = element_blockItem.namedItem("width");
        QDomNode widthVal = width.namedItem("val");
        QString str_width = widthVal.firstChild().nodeValue();
        double d_width = str_width.toDouble();
        //qDebug() << "width:" << widthVal.firstChild().nodeValue()<<endl;
        /*********************获取高************************/
        QDomNode height = element_blockItem.namedItem("height");
        QDomNode heightVal = height.namedItem("val");
        QString str_height = heightVal.firstChild().nodeValue();
        double d_height = str_height.toDouble();

        block.refid = element_blockItem.attribute("id");
        block.name = name;
        block.top_left_x = (int)x;
        block.top_left_y = (int)y;
        block.width = (int)d_width;
        block.height = (int)d_height;

        m_vecBlock.push_back(block);//将block信息全部存进一个vector容器中
    }

    // 获取所有AssociationItem节点 里面存放的是连接线相关的信息
    QDomNodeList AssociationItem = root.elementsByTagName("AssociationItem");//Block
    for (int i = 0; i < AssociationItem.count(); i++)
    {
        LineInfo line;

        // 获取链表中的值
        QDomElement element1 = AssociationItem.at(i).toElement();
        //找到需要读取的节点 Block
        qDebug() << "子节点：" << element1.nodeName();
        /*************************获取matrix中绝对位置的信息**************************/
        QDomNode matrix = element1.namedItem("matrix");
        QDomNode matrixVal = matrix.namedItem("val");
        QString matrixVal_str = matrixVal.firstChild().nodeValue();

        matrixVal_str.remove(QChar(')'), Qt::CaseInsensitive);
        QStringList matrixVal_list = matrixVal_str.split(",");//QString字符串分割函数

        QString str_x = matrixVal_list[matrixVal_list.count()-2];
        QString str_y = matrixVal_list[matrixVal_list.count()-1];
        qDebug() << "绝对位置的坐标:str_x" << str_x << "str_y: "<< str_y << endl;
        QVector<int> vec_matrix;
        vec_matrix.push_back((int)str_x.toDouble());
        vec_matrix.push_back((int)str_y.toDouble());
        line.AbsltCoordinates = QPoint((int)str_x.toDouble(),(int)str_y.toDouble());
        /*************************获取points中绝对位置的信息**************************/
        QDomNode points = element1.namedItem("points");
        QDomNode pointsVal = points.namedItem("val");
        QString pointsVal_str = pointsVal.firstChild().nodeValue();
        pointsVal_str.remove(QChar('['), Qt::CaseInsensitive);
        pointsVal_str.remove(QChar(']'), Qt::CaseInsensitive);
        pointsVal_str.remove(QChar('('), Qt::CaseInsensitive);
        pointsVal_str.remove(QChar(')'), Qt::CaseInsensitive);

        QVector<QVector<int> > vec_vecPoint;//存放这条线所有位置坐标
        vec_vecPoint.push_back(vec_matrix);
        QStringList pointsVal_List = pointsVal_str.split(",");//QString字符串分割函数
        for (int i = 0;i < pointsVal_List.size(); i+=2)
        {
            QVector<int> v;
            QString x = pointsVal_List[i];
            QString y = pointsVal_List[i+1];
            if((int)x.toDouble() == 0 && (int)y.toDouble() == 0)
            {

            }else
            {
                v.push_back((int)x.toDouble() + (int)str_x.toDouble());//最终坐标x 相对位置x+绝对位置x
                v.push_back((int)y.toDouble() + (int)str_y.toDouble());//最终坐标y 相对位置y+绝对位置y
                vec_vecPoint.push_back(v);
            }
        }
       /******************获取head-connection里面的refid的值****************/
       QDomElement head_connection = element1.firstChildElement("head-connection");
       QDomElement head_refid = head_connection.firstChildElement("ref");
       QString headRefid_str = head_refid.attribute("refid");
       line.headRefid = headRefid_str;
       //qDebug() << "head_refid:" << head_refid.attribute("refid")<< endl;

       /******************获取tail-connection里面的refid的值****************/
       QDomElement tail_connection = element1.firstChildElement("tail-connection");
       QDomElement tail_refid = tail_connection.firstChildElement("ref");
       QString tailRefid_str = tail_refid.attribute("refid");
       line.tailRefid = tailRefid_str;
       //qDebug() << "tail_refid:" << tail_refid.attribute("refid")<< endl;
       /******************获取两个节点上的端点*************************/
       int topleft_Xhead,topleft_Yhead,widthHead,heightHead;//头结点
       int topleft_XTail,topleft_YTail,widthTail,heightTail;//尾结点
        for (int i = 0; i < m_vecBlock.size(); i++)//筛选节点的位置信息
        {
            if(m_vecBlock[i].refid == headRefid_str)
            {
                topleft_Xhead = m_vecBlock[i].top_left_x;
                topleft_Yhead = m_vecBlock[i].top_left_y;
                widthHead = m_vecBlock[i].width;
                heightHead = m_vecBlock[i].height;
            }else if (m_vecBlock[i].refid == tailRefid_str)
            {
                topleft_XTail = m_vecBlock[i].top_left_x;
                topleft_YTail = m_vecBlock[i].top_left_y;
                widthTail = m_vecBlock[i].width;
                heightTail = m_vecBlock[i].height;
            }
        }
        QVector<int> vec_head,vec_tail;

        vec_head = getThePoint(vec_vecPoint,topleft_Xhead,topleft_Yhead,widthHead,heightHead);//获取头节点上连线所在的点
        vec_tail = getThePoint(vec_vecPoint,topleft_XTail,topleft_YTail,widthTail,heightTail);//获取尾节点上连线所在的点


        line.headCoordinates = QPoint(vec_head[0],vec_head[1]);
        line.tailCoordinates = QPoint(vec_tail[0],vec_tail[1]);


        m_vecline.push_back(line);
    }
    /*******************将节点含有的磁力点信息给到结构体里面**********************/
    for (int i = 0; i < m_vecline.size(); i++)
    {
        LineInfo line1 = m_vecline.at(i);
        QString headRefid1 = line1.headRefid;
        QString tailRefid1 = line1.tailRefid;
        for (int j = 0; j < m_vecBlock.size(); j++)
        {
            //BlockInfo block1 = m_vecBlock.at(j);
            if(m_vecBlock.at(j).refid == headRefid1)
            {
                QPoint megneticPoint = QPoint((line1.headCoordinates.rx() - m_vecBlock[j].top_left_x),(line1.headCoordinates.ry() - m_vecBlock[j].top_left_y));
                m_vecBlock[j].MagneticPoint.push_back(megneticPoint);
                QPoint megneticPoint_Abs = QPoint(line1.headCoordinates.rx(),line1.headCoordinates.ry());
                m_vecBlock[j].MagneticPoint_Abs.push_back(megneticPoint_Abs);
            }
            if(m_vecBlock.at(j).refid == tailRefid1)
            {
                QPoint megneticPointT = QPoint((line1.tailCoordinates.rx() - m_vecBlock[j].top_left_x),(line1.tailCoordinates.ry() - m_vecBlock[j].top_left_y));//line1.tailCoordinates;
                m_vecBlock[j].MagneticPoint.push_back(megneticPointT);
                QPoint megneticPoint_AbsT = QPoint(line1.tailCoordinates.rx() ,line1.tailCoordinates.ry());
                m_vecBlock[j].MagneticPoint_Abs.push_back(megneticPoint_AbsT);
            }
        }

    }
    /************************创建节点******************************/
    for (int i = 0; i < m_vecBlock.size(); i++)
    {
        BlockInfo block = m_vecBlock.at(i);
        m_vecBlock[i].SerialNumber = i;
        createNode(block.name,block.top_left_x,block.top_left_y,block.width,block.height,block.MagneticPoint);
    }
    /************************创建线***********************/
    for (int i = 0; i < m_vecline.size(); i++)
    {
        LineInfo line = m_vecline.at(i);
        Chart_Base * head;
        Chart_Base * tail;
        int mgtcHIndex;
        int mgtcTIndex;
        QPoint headPoint = line.headCoordinates;
        QPoint tailPoint = line.tailCoordinates;

        BlockInfo headBloak,tailBlock;
        for (int j = 0; j < m_vecBlock.size(); j++)
        {
           int num = m_vecBlock.at(j).SerialNumber;

           if(line.headRefid == m_vecBlock.at(j).refid)
           {

               head = chart_xml.at(num);
               headBloak =  m_vecBlock.at(j);
           }
           if(line.tailRefid == m_vecBlock.at(j).refid)
           {

                tail = chart_xml.at(num);
                tailBlock = m_vecBlock.at(j);
            }

        }
        for (int i = 0;i < headBloak.MagneticPoint_Abs.size(); i++)
        {
            if(headPoint == headBloak.MagneticPoint_Abs.at(i))
            {
                mgtcHIndex = i;
            }
        }

        for (int i = 0;i < tailBlock.MagneticPoint_Abs.size(); i++)
        {
            if(tailPoint == tailBlock.MagneticPoint_Abs.at(i))
            {
                mgtcTIndex = i;
            }
        }

        createLine(head,line.headCoordinates,mgtcHIndex,tail,line.tailCoordinates,mgtcTIndex);

    }

    // 关联后可以关闭文件了
    file.close();

    return true;
}
void FlowChart::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setPen(QColor(150,150,150));
    const int gap = 30;
    for(int i = 1;i * gap < width();i++)
    {
        if(i%5 == 0)
            p.drawLine(QLineF(i*gap,0,i*gap,height()));
    }
    for(int i = 1;i * gap < height();i++)
    {
        if(i%5 == 0)
            p.drawLine(QLineF(0,i*gap,width(),i*gap));
    }

    p.setPen(QColor(200,200,200,150));
    //p.setPen(Qt::DotLine);
    //p.setOpacity(200);
    for(int i = 1;i * gap < width();i++)
    {
        if(i%5 == 0) continue;
        p.drawLine(QLineF(i*gap,0,i*gap,height()));
    }
    for(int i = 1;i * gap < height();i++)
    {
        if(i%5 == 0) continue;
        p.drawLine(QLineF(0,i*gap,width(),i*gap));
    }


    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);

    //event->accept();
}
void FlowChart::mousePressEvent(QMouseEvent *event)
{

    event->ignore();
    switch(mouseEventType)
    {
        case MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE:{
            int x = event->pos().rx();
            int y = event->pos().ry();
            //curPaintChart->setObjectName(QStringLiteral("tttt"));
            if(curSelecChart) curSelecChart->hideMagSize();
            curSelecChart = curPaintChart;
            curPaintChart->setXY(x,y);
            //curPaintChart->setAttribute(Qt::WA_StyledBackground,true);
            //curPaintChart->setStyleSheet(QStringLiteral("background-color: rgb(100, 100, 100);"));
            if(curPaintChart->chartType == PaintChartType::LINE)
            {
                connect(curPaintChart,SIGNAL(sendThisClass(Chart_Base *, int,int)),this,SLOT(setSelecChart(Chart_Base *, int,int)));
                connect(curPaintChart,SIGNAL(setTypeChangeSize(ORIENTION)),this,SLOT(setTypeChangeSize(ORIENTION)));
                addLine(curPaintChart);
                Chart_Line *cl = dynamic_cast<Chart_Line*>(curSelecChart);
                emit sendLineStyle(cl->paintChartDrawPen,cl->getStartLineHeadType(),cl->getEndLineHeadType());
            }else
            {
                connect(curPaintChart,SIGNAL(sendThisClass(Chart_Base *, int,int)),this,SLOT(setSelecChart(Chart_Base *, int,int)));
                connect(curPaintChart,SIGNAL(setTypeChangeSize(ORIENTION)),this,SLOT(setTypeChangeSize(ORIENTION)));
                connect(curPaintChart,SIGNAL(setTypeCreateMagPoint(Chart_Base *,ORIENTION,int)),this,SLOT(setTypeCreateMagPoint(Chart_Base *,ORIENTION,int)));
                //connect(curPaintChart,SIGNAL(hideThisClass()),this,SLOT(resetSeletcChart()));
                addChart(curPaintChart);
                emit sendChartStyle(curSelecChart->paintChartDrawPen,curSelecChart->paintChartFillPen);
            }
            curPaintChart->update();
            curPaintChart->show();
            //mouseEventType = MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE;
            event->accept();
        }break;
        default:case MOUSE_EVENT_TYPE::NONE:{
            for(auto it = charts.begin();it!=charts.end();it++)
            {
                (*it)->overlapChartMousePressed(event);
                if(event->isAccepted()) return;
            }
            for(auto a:line)
            {
                a->overlapChartMousePressed(event);
                if(event->isAccepted()) return;
            }
            hideMagSizeAll();
        }break;
    }
}
void FlowChart::mouseMoveEvent(QMouseEvent *event)
{
    //if(event->isAccepted()) return;
    event->ignore();


    switch(mouseEventType)
    {
        default:case MOUSE_EVENT_TYPE::NONE:
        {
            for(auto it = charts.begin();it!=charts.end();it++)
            {
                (*it)->overlapChartMouseMove(event);
                if(event->isAccepted()) return;
            }
            for(auto it = line.begin();it!=line.end();it++)
            {
                (*it)->overlapChartMouseMove(event);
                if(event->isAccepted()) return;
            }
            if(curSelecChart == nullptr)
            {
                setCursor(QCursor(Qt::ArrowCursor));
                hideMagSizeAll();
            }
        }break;
        case MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE:
        {
            int x = event->pos().rx(),y = event->pos().ry();
            if(curSelecChart)
            {
                if(curSelecChart->chartType == PaintChartType::LINE)
                {
                    for(auto it = charts.begin();it!=charts.end();it++)
                    {
                        if((*it)->autoSetMagi(x,y,magPointIndex))
                        {
                            lineSelectChart = *it;
                            break;
                        }else{
                            lineSelectChart = nullptr;
                        }
                    }
                    //if(!lineSelectChart) reinterpret_cast<Chart_Line *>(curSelecChart)->resetEndChart();
                }
//                if(curSelecChart->chartType == PaintChartType::RECT)
//                {
                    //emit signalTest();
                //}
                curPaintChart->setWidthHeight(x,y);


            }
            event->accept();
        }break;
        case MOUSE_EVENT_TYPE::RUNTIME_CHANGE_SIZE:
        {
            int x = event->pos().rx(),y = event->pos().ry();

            if(curSelecChart->chartType == PaintChartType::LINE)
            {
                //qDebug()<<"hehe";
                for(auto it = charts.begin();it!=charts.end();it++)
                {
                    if((*it)->autoSetMagi(x,y,magPointIndex))
                    {
                        lineSelectChart = *it;
                        break;
                    }else{
                        lineSelectChart = nullptr;
                    }
                }
                //if(!lineSelectChart) reinterpret_cast<Chart_Line *>(curSelecChart)->resetEndChart();
            }
//            if(curSelecChart->chartType == PaintChartType::RECT)
//            {
                //emit signalTest();
            //}
            curSelecChart->setWidthHeight(x,y,sizePointDirect);

            event->accept();
        }break;
        case MOUSE_EVENT_TYPE::RUNTIME_CREATE_MAGPOINT:
        {
            if(newLineChart == nullptr)
            {
                newLineChart = new Chart_Line(this);
                if(curSelecChart)
                    curSelecChart->hideMagSize();
                curSelecChart = newLineChart;
                addLine(newLineChart);
                //newLineChart->setAttribute(Qt::WA_StyledBackground,true);
                //newLineChart->setStyleSheet(QStringLiteral("background-color: rgb(100, 100, 100);"));
                connect(newLineChart,SIGNAL(sendThisClass(Chart_Base *, int,int)),this,SLOT(setSelecChart(Chart_Base *, int,int)));
                connect(newLineChart,SIGNAL(setTypeChangeSize(ORIENTION)),this,SLOT(setTypeChangeSize(ORIENTION)));
                newLineChart->setXY(newLineFromSelectChart->getMagiPointAbsX(magPointFromIndex),newLineFromSelectChart->getMagiPointAbsY(magPointFromIndex));
                newLineChart->setStartChart(newLineFromSelectChart);
                newLineChart->setStartMagIndex(magPointFromIndex);
                newLineChart->setStartDirect(magPointDirect);
                newLineChart->update();
                newLineChart->show();

                newLineFromSelectChart->addMagiPointStartLine(magPointFromIndex,newLineChart);
            }
            int x = event->pos().rx(),y = event->pos().ry();

            for(auto it = charts.begin();it!=charts.end();it++)
            {
                //if(*it == newLineFromSelectChart) continue;
                if((*it)->autoSetMagi(x,y,magPointToIndex))
                {
                    newLineToSelectChart = *it;
                    break;
                }else{
                    newLineToSelectChart = nullptr;
                }
            }
            //if(!newLineToSelectChart) newLineChart->resetEndChart();
            newLineChart->setWidthHeight(x,y,ORIENTION::ENDPOINT);
            event->ignore();
        }break;
        case MOUSE_EVENT_TYPE::RUNTIME_CHANGE_POS:
        {
            if(curSelecChart->chartType != PaintChartType::LINE)
            {

                //curSelecChart
                curSelecChart->setXY(event->pos().rx() - curSelecChartPos.rx(),event->pos().ry() - curSelecChartPos.ry());
                event->accept();

                curSelecChartStart = curSelecChart->GetWightStart();
                //qDebug() << "curSelecChart->GetWightStart()" << recv.rx() << endl ;
                curSelecChartEnd = curSelecChart->GetWightEnd();
                //qDebug() << "curSelecChart->GetWightEnd()" << recv1.rx() << endl ;


            }


        }break;
    }
}

void FlowChart::mouseReleaseEvent(QMouseEvent *event)
{
    switch(mouseEventType)
    {
        case MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE:
        {
            //curPaintChart->updatePath();
            resetPaintChartType();
            if(curSelecChart->chartType == PaintChartType::LINE)
            {
                Chart_Line *cl = reinterpret_cast<Chart_Line *>(curSelecChart);

                if(lineSelectChart)
                {
                    lineSelectChart->addMagiPointEndLine(magPointIndex,curSelecChart);
                    lineSelectChart->hideMagOnly();
                    cl->setEndChart(lineSelectChart);
                    cl->setEndMagIndex(magPointIndex);
                    cl->setEndDirect(lineSelectChart->getMagiPointDirect(magPointIndex));
                    cl->update();
                    lineSelectChart = nullptr;

                    setLineStartStyle(m_startStyle);
                    setLineEndStyle(m_endStyle);

//                    m_startStyle = 0;
//                    m_endStyle = 0;

                }
                else{
                    cl->resetEndChart();
                }

            }


            curPaintChart = nullptr;
            mouseEventType = MOUSE_EVENT_TYPE::NONE;
            setFileSetSaved(false);
            event->accept();
        }break;
        case MOUSE_EVENT_TYPE::RUNTIME_CHANGE_SIZE:
        {
            if(curSelecChart->chartType == PaintChartType::LINE)
            {
                Chart_Line *cl = reinterpret_cast<Chart_Line *>(curSelecChart);
                if(lineSelectChart)
                {
                    if(sizePointDirect == ORIENTION::STARTPOINT)
                    {
                        cl->resetStartChart();
                        lineSelectChart->addMagiPointStartLine(magPointIndex,curSelecChart);
                        lineSelectChart->hideMagOnly();
                        cl->setStartChart(lineSelectChart);
                        cl->setStartMagIndex(magPointIndex);
                        cl->setStartDirect(lineSelectChart->getMagiPointDirect(magPointIndex));
                    }else if(sizePointDirect == ORIENTION::ENDPOINT)
                    {
                        cl->resetEndChart();
                        lineSelectChart->addMagiPointEndLine(magPointIndex,curSelecChart);
                        lineSelectChart->hideMagOnly();
                        cl->setEndChart(lineSelectChart);
                        cl->setEndMagIndex(magPointIndex);
                        cl->setEndDirect(lineSelectChart->getMagiPointDirect(magPointIndex));
                    }else{
                        qDebug()<<"Error";
                    }

                    cl->update();
                }else
                {
                    if(sizePointDirect == ORIENTION::STARTPOINT)
                    {
                        cl->resetStartChart();
                    }else if(sizePointDirect == ORIENTION::ENDPOINT)
                    {
                        cl->resetEndChart();
                    }else
                    {
                        qDebug()<<"Error";
                    }
                }
            }
            mouseEventType = MOUSE_EVENT_TYPE::NONE;
            lineSelectChart = nullptr;
            setFileSetSaved(false);
            event->accept();
        }break;
        case MOUSE_EVENT_TYPE::RUNTIME_CREATE_MAGPOINT:
        {
            if(newLineToSelectChart)
            {
                newLineToSelectChart->addMagiPointEndLine(magPointToIndex,newLineChart);
                newLineToSelectChart->hideMagOnly();
                newLineChart->setEndChart(newLineToSelectChart);
                newLineChart->setEndMagIndex(magPointToIndex);
                newLineChart->setEndDirect(newLineToSelectChart->getMagiPointDirect(magPointToIndex));
                newLineChart->update();
            }else
            {
                if(newLineChart)
                    newLineChart->resetEndChart();
            }
            mouseEventType = MOUSE_EVENT_TYPE::NONE;
            newLineChart = nullptr;
            newLineFromSelectChart = nullptr;
            newLineToSelectChart = nullptr;
            setFileSetSaved(false);
            event->accept();
        }break;
        case MOUSE_EVENT_TYPE::RUNTIME_CHANGE_POS:
        {
            mouseEventType = MOUSE_EVENT_TYPE::NONE;
            setFileSetSaved(false);
            event->accept();
        }break;
        default:case MOUSE_EVENT_TYPE::NONE:
        {
            event->ignore();
        }break;
    }

}
void FlowChart::keyPressEvent(QKeyEvent *ev)
{
//    ev->ignore();
//    switch(ev->key())
//    {
//        case Qt::Key_Escape:
//        {
//            if(curSelecChart)
//            {
//                curSelecChart->hideMagSize();
//                curSelecChart = nullptr;
//            }
//        }break;
//        case Qt::Key_Delete:
//        {
//            if(curSelecChart)
//            {
//                if(curSelecChart->chartType == PaintChartType::LINE)
//                {
//                    if(!delLine(curSelecChart))
//                    {
//                        qDebug()<<"Error";
//                    }
//                }else{
//                    if(!delChart(curSelecChart))
//                    {
//                        qDebug()<<"Error";
//                    }
//                }
//                curSelecChart = nullptr;
//            }
//        }break;

#if 0
        case Qt::Key_Q:
        {
            qDebug()<<"总个数：charts:"<<charts.size()<<",lines:"<<line.size();
            for(auto it = charts.begin();it != charts.end();it++)
            {
                qDebug()<<"\t磁力点个数："<<(*it)->magPoint.i_point.size();
                for(auto magit = (*it)->magPoint.i_point.begin();magit!=(*it)->magPoint.i_point.end();magit++)
                {
                    qDebug()<<"\t\t线头个数："<<(*magit)->i_lineStart.size()<<"，线尾个数："<<(*magit)->i_lineEnd.size();
                    for(auto magLineStIt = (*magit)->i_lineStart.begin();magLineStIt != (*magit)->i_lineStart.end();magLineStIt++)
                    {
                        qDebug()<<"\t\t\t线头ID："<<(*magLineStIt)->getID();
                    }
                    for(auto magLineEnIt = (*magit)->i_lineEnd.begin();magLineEnIt != (*magit)->i_lineEnd.end();magLineEnIt++)
                    {
                        qDebug()<<"\t\t\t线尾ID："<<(*magLineEnIt)->getID();
                    }
                }
            }
            qDebug();
            for(auto it = line.begin();it != line.end();it++)
            {
                qDebug()<<"\t线头："<<((dynamic_cast<Chart_Line*>(*it)->getStartChart() == nullptr)?"-":"有")<<"，线尾："<<(((dynamic_cast<Chart_Line*>(*it)->getEndChart()) == nullptr)?"-":"有");
            }
        }break;
#endif
//        default:{
//            ev->ignore();
//        }
//    }
}
void FlowChart::keyReleaseEvent(QKeyEvent *ev)
{
    ev->ignore();
}
void FlowChart::deleteChart_base()
{
    QMessageBox msgBox;
   if(curSelecChart)
    {
        if(curSelecChart->chartType == PaintChartType::LINE)
        {
            if(!delLine(curSelecChart))
            {

                qDebug()<<"Error";
                msgBox.setText("删除连线失败!!!");
            }
        }else
        {
            if(!delChart(curSelecChart))
            {
                qDebug()<<"Error";
                msgBox.setText("删除节点失败!!!");

            }
        }


        curSelecChart = nullptr;


        msgBox.setText("删除成功!!!");


    }else
    {
       if(btn_Select)
        {
            btn_Select->deleteLater();//删除按钮
            btn_Select = nullptr;
            msgBox.setText("删除按钮成功!!!");
            //msgBox.exec();
        }else
        {
           msgBox.setText("没有控件被选中!!!");
           //msgBox.exec();
        }


    }
    msgBox.exec();

//else
//   {
//       msgBox.setText("没有选中控件哦!!!");
//       msgBox.exec();
//   }

}
//修改颜色为红色
void FlowChart::recvModifyColor_Red()
{
    if(curSelecChart)
    {
        if(curSelecChart->chartType == PaintChartType::LINE)
        {
            //qDebug() << "来修改线颜色哈!!!" << endl;
            QColor color;
            color.setRgb(255,0,0);

            setSelChartLineColor(color);
        }else
        {
            //qDebug() << "来修改颜色哈!!!" << endl;
            QColor color;
            color.setRgb(255,0,0);
            setSelChartFillColor(color);
        }

    }
}
//修改颜色为绿色
void FlowChart::recvModifyColor_Green()
{
    if(curSelecChart)
    {
        if(curSelecChart->chartType == PaintChartType::LINE)
        {
            //qDebug() << "来修改线颜色哈!!!" << endl;
            QColor color;
            color.setRgb(0,255,0);

            setSelChartLineColor(color);
        }else
        {
            //qDebug() << "来修改颜色哈!!!" << endl;
            QColor color;
            color.setRgb(0,255,0);
            setSelChartFillColor(color);
        }

    }
}
//修改颜色为蓝色
void FlowChart::recvModifyColor_Blue()
{
    if(curSelecChart)
    {
        if(curSelecChart->chartType == PaintChartType::LINE)
        {
            //qDebug() << "来修改线颜色哈!!!" << endl;
            QColor color;
            color.setRgb(0,0,255);

            setSelChartLineColor(color);
        }else
        {
            //qDebug() << "来修改颜色哈!!!" << endl;
            QColor color;
            color.setRgb(0,0,255);
            setSelChartFillColor(color);
        }

    }
}
//修改颜色为白色
void FlowChart::recvModifyColor_White()
{
    if(curSelecChart)
    {
        if(curSelecChart->chartType == PaintChartType::LINE)
        {
            //qDebug() << "来修改线颜色哈!!!" << endl;
            QColor color;
            color.setRgb(255,255,255);

            setSelChartLineColor(color);
        }else
        {
            //qDebug() << "来修改颜色哈!!!" << endl;
            QColor color;
            color.setRgb(255,255,255);
            setSelChartFillColor(color);
        }

    }
}
void FlowChart::MyTest()
{

    curPaintChart = new Chart_Rect(this);
    if(curPaintChart)
    {
        mouseEventType = MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE;
    }
    int x = 100;
    int y = 100;

    if(curSelecChart) curSelecChart->hideMagSize();
    curSelecChart = curPaintChart;
    curPaintChart->setXY(x,y);

    connect(curPaintChart,SIGNAL(sendThisClass(Chart_Base *, int,int)),this,SLOT(setSelecChart(Chart_Base *, int,int)));
    connect(curPaintChart,SIGNAL(setTypeChangeSize(ORIENTION)),this,SLOT(setTypeChangeSize(ORIENTION)));
    connect(curPaintChart,SIGNAL(setTypeCreateMagPoint(Chart_Base *,ORIENTION,int)),this,SLOT(setTypeCreateMagPoint(Chart_Base *,ORIENTION,int)));
    //connect(curPaintChart,SIGNAL(hideThisClass()),this,SLOT(resetSeletcChart()));
    addChart(curPaintChart);
    emit sendChartStyle(curSelecChart->paintChartDrawPen,curSelecChart->paintChartFillPen);
    curPaintChart->update();
    curPaintChart->show();
    //mouseEventType = MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE;

    int x1 = 200,y1 = 230;
    if(curSelecChart)
    {


        curPaintChart->setWidthHeight(x1,y1);
    }

    QFont ft;
    ft.setPointSize(20);
    ft.setBold(true);

    //connect(this,SIGNAL(signalTest()),curPaintChart,SLOT(buttonClicked()));
    //第二种创建按钮
    btn2 =new QPushButton("+",curPaintChart);
    //移动窗口
    btn2->setFont(ft);
    btn2->setStyleSheet("QPushButton{color:red;background:white}");

    btn2->move((x1-x)/2-9,y1-y-24);//btn2->move((x1-x)/2+5,y1-y-10);//边缘是20的时候
    //重置窗口大小
    //resize(960,640);
    btn2->show();
    //btn可不可以resize？可以
    btn2->resize(30,30);

    connect(btn2,SIGNAL(clicked()),m_SelectImgForm,SLOT(slot_ShowWidget()));

    //connect(m_SelectImgForm,SIGNAL(signal_SendImgPath(QSting)),this,SLOT(slot_RecvImagePath(QString)));


    resetPaintChartType();


    curPaintChart = nullptr;
    mouseEventType = MOUSE_EVENT_TYPE::NONE;
    setFileSetSaved(false);

}
void FlowChart::createBlock()
{
    curPaintChart = new Chart_Block(this);
    if(curPaintChart)
    {
        mouseEventType = MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE;
    }
    int x = 100;
    int y = 100;

    if(curSelecChart) curSelecChart->hideMagSize();
    curSelecChart = curPaintChart;
    curPaintChart->setXY(x,y);

    connect(curPaintChart,SIGNAL(sendThisClass(Chart_Base *, int,int)),this,SLOT(setSelecChart(Chart_Base *, int,int)));
    connect(curPaintChart,SIGNAL(setTypeChangeSize(ORIENTION)),this,SLOT(setTypeChangeSize(ORIENTION)));
    connect(curPaintChart,SIGNAL(setTypeCreateMagPoint(Chart_Base *,ORIENTION,int)),this,SLOT(setTypeCreateMagPoint(Chart_Base *,ORIENTION,int)));
    //connect(curPaintChart,SIGNAL(hideThisClass()),this,SLOT(resetSeletcChart()));
    addChart(curPaintChart);
    emit sendChartStyle(curSelecChart->paintChartDrawPen,curSelecChart->paintChartFillPen);
    curPaintChart->update();
    curPaintChart->show();
    //mouseEventType = MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE;

    int x1 = 200,y1 = 230;
    if(curSelecChart)
    {


        curPaintChart->setWidthHeight(x1,y1);
    }

//    QFont ft;
//    ft.setPointSize(20);
//    ft.setBold(true);
//    //第二种创建按钮
//    btn2 =new QPushButton("+",curPaintChart);
//    //移动窗口
//    btn2->setFont(ft);
//    btn2->setStyleSheet("QPushButton{color:red;background:white}");

//    btn2->move((x1-x)/2-9,y1-y-24);//btn2->move((x1-x)/2+5,y1-y-10);//边缘是20的时候
//    //重置窗口大小
//    //resize(960,640);
//    btn2->show();
//    //btn可不可以resize？可以
//    btn2->resize(30,30);

//    connect(btn2,SIGNAL(clicked()),m_SelectImgForm,SLOT(slot_ShowWidget()));



    resetPaintChartType();


    curPaintChart = nullptr;
    mouseEventType = MOUSE_EVENT_TYPE::NONE;
    setFileSetSaved(false);
}
//新建表示组合的线
void FlowChart::createLine_Combination()
{
    if(curPaintChart != nullptr)
        delete curPaintChart;

    curPaintChart = new Chart_Line(this);
    if(curPaintChart)
    {
        mouseEventType = MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE;
    }

    m_startStyle = 4;//开始方向的箭头类型
    m_endStyle = 1;//开始方向的箭头类型
    //m_endStyle = LINE_HEAD_TYPE::ARROW0;//结束方向的箭头类型

//    setLineStartStyle(4);






}
//新建表示引用的线
void FlowChart::createLine_Quote()
{
    qDebug() << "FlowChart::createLine_Quote()" << endl;
    if(curPaintChart != nullptr)
        delete curPaintChart;

    curPaintChart = new Chart_Line(this);
    if(curPaintChart)
    {
        mouseEventType = MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE;
    }

    //m_startStyle = 4;//开始方向的箭头类型
    m_startStyle = 1;//开始方向的箭头类型
    m_endStyle = 0;

}
//创建表示泛化的线
void FlowChart::createLine_Generalization()
{
    qDebug() << "FlowChart::createLine_Quote()" << endl;
    if(curPaintChart != nullptr)
        delete curPaintChart;

    curPaintChart = new Chart_Line(this);
    if(curPaintChart)
    {
        mouseEventType = MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE;
    }

    m_startStyle = 2;//开始方向的箭头类型
    m_endStyle = 0;
}
void FlowChart::slot_RecvImagePath(QString path)
{
    /*
    //QPoint start = curPaintChart->GetWightStart();
//    qDebug() << curSelecChartStart.rx() << curSelecChartStart.ry() << endl;
//    qDebug() << curSelecChartEnd.rx() << curSelecChartEnd.ry() << endl;
    qDebug() << "FlowChart::slot_RecvImagePath(QString path)::" <<path<<  endl;*/
    /******************生成button的代码************************/

    Pb_generatedPb =new QPushButton("",this);

    //resize(960,640);
    Pb_generatedPb->show();
    //btn可不可以resize？可以
    Pb_generatedPb->resize(100,100);
    Pb_generatedPb->setStyleSheet(path);//图片在资源文件中
    //Pb_generatedPb->move(200,200);
    Pb_generatedPb->move(curSelecChartStart.rx(),curSelecChartEnd.ry()+6);//btn2->move((x1-x)/2-30,y1-y-25);//
    m_btnGroup->addButton(Pb_generatedPb);
    m_btnGroup->setId(Pb_generatedPb,m_BtnID);
    btn2->setChecked(true);
    m_BtnID += 1;

    curPaintChart = nullptr;

}
void FlowChart::slot_clickedGroup(int num)
{
    qDebug() << "MainWindow::slot_clickedGroup::" << num << endl;

    btn_Select = qobject_cast<QPushButton*> (m_btnGroup->button(num));



}
//加载外部文件
void FlowChart::loadExternalFiles()
{
    //qDebug() << "FlowChart::loadExternalFiles()" << endl;
    // 打开文件

    QFile file("D:/qt5_12_1Pro/FlowChartTools-master_17_X64/package0.gaphor");//tx
    if (!file.open(QFileDevice::ReadOnly))
    {
        qDebug() << "文件打开失败!!!" << endl;
        //QMessageBox::information(this, "提示", "文件打开失败！");
        //return;
    }

    QDomDocument doc;

    // 将doc与file关联起来
    // 这个函数从IO设备dev中读取XML文档，如果内容被成功解析，则返回true;否则返回false。
    if (!doc.setContent(&file))
    {
        qDebug() << "操作的不是xml文件!!!" << endl;
//        QMessageBox::information(NULL, "提示", "操作的文件不是XML文件！");
//        file.close();
        //return;
    }else
    {
        qDebug() << "是xml文件!!!" << endl;
    }
    // 关联后可以关闭文件了
    //file.close();
    QDomElement root = doc.documentElement();



    // 获取所有Book1节点
    QDomNodeList list1 = root.elementsByTagName("Block");//Block
    for (int i = 0; i < list1.count(); i++)
    {
        // 获取链表中的值
        QDomElement element1 = list1.at(i).toElement();
        //找到需要读取的节点 Block
        //qDebug() << "子节点：" << element1.nodeName();
        // 获得子节点
        QDomNode name = element1.namedItem("name");
        QDomNode nameVal = name.namedItem("val");
        QString nameString = nameVal.firstChild().nodeValue();
        //qDebug() << "Name:" << nameString <<endl;

            /******************获取package里面的refid的值****************/
//            QDomElement package = element1.firstChildElement("package");
//            QDomElement refid = package.firstChildElement("ref");
//            qDebug() << "ref:" << refid.attribute("refid")<< endl;

            /******************获取presentation里面的reflist里面的ref refid的值*********/
        QDomElement presentation = element1.firstChildElement("presentation");
        QDomElement reflist = presentation.firstChildElement("reflist");
        QDomElement ref = reflist.firstChildElement("ref");
        QString refid =  ref.attribute("refid");
        qDebug() << "refid:" << refid << endl;
        nodeNameAndID.insert(refid,nameString);

    }

    // 获取所有BlockItem节点
    QDomNodeList list_BlockItem = root.elementsByTagName("BlockItem");//Block
    for (int i = 0; i < list_BlockItem.count(); i++)
    {
        //m_vecBlock
        BlockInfo block;
        // 获取链表中的值
        QDomElement element_blockItem= list_BlockItem.at(i).toElement();
        //找到需要读取的节点 Block

        // 通过attribute方法返回属性中的值
        QString name = nodeNameAndID.value(element_blockItem.attribute("id"));

        qDebug() << "子节点：" << element_blockItem.nodeName() << "  Name:" << name ;
        // 获得子节点
        QDomNode matrix = element_blockItem.namedItem("matrix");
        QDomNode matrixVal = matrix.namedItem("val");
        QString matrixVal_str = matrixVal.firstChild().nodeValue();
        //qDebug() << "matrixVal:" << matrixVal_str <<endl;
        //t.remove(QChar('\n'), Qt::CaseInsensitive);
        matrixVal_str.remove(QChar(')'), Qt::CaseInsensitive);
        QStringList matrixVal_list = matrixVal_str.split(",");//QString字符串分割函数

        QString str_x = matrixVal_list[matrixVal_list.count()-2];
        QString str_y = matrixVal_list[matrixVal_list.count()-1];
        //qDebug() << "str_x:" << str_x << "str_y:" << str_y <<endl;
        /*********************获得子左上角坐标***********************/

        QDomNode top_left = element_blockItem.namedItem("top-left");
        QDomNode top_leftVal = top_left.namedItem("val");
        QString top_leftValstr = top_leftVal.firstChild().nodeValue();
        top_leftValstr.remove(QChar('('), Qt::CaseInsensitive);
        top_leftValstr.remove(QChar(')'), Qt::CaseInsensitive);
        QStringList top_leftValList = top_leftValstr.split(",");//QString字符串分割函数
        QString str_leftx = top_leftValList[0];
        QString str_lefty = top_leftValList[1];
        //qDebug() << "str_leftx:" << str_leftx << "str_lefty:" << str_lefty <<endl;
        double x = str_leftx.toDouble() + str_x.toDouble();//左上角坐标是top-left里val的x+matrix里val的x
        double y = str_lefty.toDouble() + str_y.toDouble();
        //qDebug() << "x:" << x << "y:" <<  y << endl;
        /*********************获取宽************************/
        QDomNode width = element_blockItem.namedItem("width");
        QDomNode widthVal = width.namedItem("val");
        QString str_width = widthVal.firstChild().nodeValue();
        double d_width = str_width.toDouble();
        //qDebug() << "width:" << widthVal.firstChild().nodeValue()<<endl;
        /*********************获取高************************/
        QDomNode height = element_blockItem.namedItem("height");
        QDomNode heightVal = height.namedItem("val");
        QString str_height = heightVal.firstChild().nodeValue();
        double d_height = str_height.toDouble();
        //qDebug() << "heightVal:" << heightVal.firstChild().nodeValue()<<endl;

        //createNode(name,(int)x,(int)y,(int)d_width,(int)d_height);
        block.refid = element_blockItem.attribute("id");
        block.name = name;
        block.top_left_x = (int)x;
        block.top_left_y = (int)y;
        block.width = (int)d_width;
        block.height = (int)d_height;

        m_vecBlock.push_back(block);//将block信息全部存进一个vector容器中
    }

    // 获取所有AssociationItem节点 里面存放的是连接线相关的信息
    QDomNodeList AssociationItem = root.elementsByTagName("AssociationItem");//Block
    for (int i = 0; i < AssociationItem.count(); i++)
    {
        LineInfo line;

        // 获取链表中的值
        QDomElement element1 = AssociationItem.at(i).toElement();
        //找到需要读取的节点 Block
        qDebug() << "子节点：" << element1.nodeName();
        /*************************获取matrix中绝对位置的信息**************************/
        QDomNode matrix = element1.namedItem("matrix");
        QDomNode matrixVal = matrix.namedItem("val");
        QString matrixVal_str = matrixVal.firstChild().nodeValue();

        matrixVal_str.remove(QChar(')'), Qt::CaseInsensitive);
        QStringList matrixVal_list = matrixVal_str.split(",");//QString字符串分割函数

        QString str_x = matrixVal_list[matrixVal_list.count()-2];
        QString str_y = matrixVal_list[matrixVal_list.count()-1];
        qDebug() << "绝对位置的坐标:str_x" << str_x << "str_y: "<< str_y << endl;
        QVector<int> vec_matrix;
        vec_matrix.push_back((int)str_x.toDouble());
        vec_matrix.push_back((int)str_y.toDouble());
        line.AbsltCoordinates = QPoint((int)str_x.toDouble(),(int)str_y.toDouble());
        /*************************获取points中绝对位置的信息**************************/
        QDomNode points = element1.namedItem("points");
        QDomNode pointsVal = points.namedItem("val");
        QString pointsVal_str = pointsVal.firstChild().nodeValue();
        pointsVal_str.remove(QChar('['), Qt::CaseInsensitive);
        pointsVal_str.remove(QChar(']'), Qt::CaseInsensitive);
        pointsVal_str.remove(QChar('('), Qt::CaseInsensitive);
        pointsVal_str.remove(QChar(')'), Qt::CaseInsensitive);

        QVector<QVector<int> > vec_vecPoint;//存放这条线所有位置坐标
        vec_vecPoint.push_back(vec_matrix);
        QStringList pointsVal_List = pointsVal_str.split(",");//QString字符串分割函数
        for (int i = 0;i < pointsVal_List.size(); i+=2)
        {
            QVector<int> v;
            QString x = pointsVal_List[i];
            QString y = pointsVal_List[i+1];
            if((int)x.toDouble() == 0 && (int)y.toDouble() == 0)
            {

            }else
            {
                v.push_back((int)x.toDouble() + (int)str_x.toDouble());//最终坐标x 相对位置x+绝对位置x
                v.push_back((int)y.toDouble() + (int)str_y.toDouble());//最终坐标y 相对位置y+绝对位置y
                vec_vecPoint.push_back(v);
            }


        }

        //qDebug() << "pointsVal_str" << pointsVal_str << endl;
       /******************获取head-connection里面的refid的值****************/
       QDomElement head_connection = element1.firstChildElement("head-connection");
       QDomElement head_refid = head_connection.firstChildElement("ref");
       QString headRefid_str = head_refid.attribute("refid");
       line.headRefid = headRefid_str;
       //qDebug() << "head_refid:" << head_refid.attribute("refid")<< endl;

       /******************获取tail-connection里面的refid的值****************/
       QDomElement tail_connection = element1.firstChildElement("tail-connection");
       QDomElement tail_refid = tail_connection.firstChildElement("ref");
       QString tailRefid_str = tail_refid.attribute("refid");
       line.tailRefid = tailRefid_str;
       //qDebug() << "tail_refid:" << tail_refid.attribute("refid")<< endl;
       /******************获取两个节点上的端点*************************/
       int topleft_Xhead,topleft_Yhead,widthHead,heightHead;//头结点
       int topleft_XTail,topleft_YTail,widthTail,heightTail;//尾结点
        for (int i = 0; i < m_vecBlock.size(); i++)//筛选节点的位置信息
        {
            if(m_vecBlock[i].refid == headRefid_str)
            {
                topleft_Xhead = m_vecBlock[i].top_left_x;
                topleft_Yhead = m_vecBlock[i].top_left_y;
                widthHead = m_vecBlock[i].width;
                heightHead = m_vecBlock[i].height;
            }else if (m_vecBlock[i].refid == tailRefid_str)
            {
                topleft_XTail = m_vecBlock[i].top_left_x;
                topleft_YTail = m_vecBlock[i].top_left_y;
                widthTail = m_vecBlock[i].width;
                heightTail = m_vecBlock[i].height;
            }
        }


        //m_lineBlock.push_back();
        QVector<int> vec_head,vec_tail;
        //QVector<QVector<int> > vecVec,int topLeftx,int topLefty,int width,int height
        vec_head = getThePoint(vec_vecPoint,topleft_Xhead,topleft_Yhead,widthHead,heightHead);//获取头节点上连线所在的点
        vec_tail = getThePoint(vec_vecPoint,topleft_XTail,topleft_YTail,widthTail,heightTail);//获取尾节点上连线所在的点
        //createLine(vec_head[0],vec_head[1],vec_tail[0],vec_tail[1]);

        line.headCoordinates = QPoint(vec_head[0],vec_head[1]);
        line.tailCoordinates = QPoint(vec_tail[0],vec_tail[1]);
        //createLine(line.headCoordinates,line.tailCoordinates);

        m_vecline.push_back(line);
    }
    /*******************将节点含有的磁力点信息给到结构体里面**********************/
    for (int i = 0; i < m_vecline.size(); i++)
    {
        LineInfo line1 = m_vecline.at(i);
        QString headRefid1 = line1.headRefid;
        QString tailRefid1 = line1.tailRefid;
        for (int j = 0; j < m_vecBlock.size(); j++)
        {
            //BlockInfo block1 = m_vecBlock.at(j);
            if(m_vecBlock.at(j).refid == headRefid1)
            {
                QPoint megneticPoint = QPoint((line1.headCoordinates.rx() - m_vecBlock[j].top_left_x),(line1.headCoordinates.ry() - m_vecBlock[j].top_left_y));
                m_vecBlock[j].MagneticPoint.push_back(megneticPoint);
                QPoint megneticPoint_Abs = QPoint(line1.headCoordinates.rx(),line1.headCoordinates.ry());
                m_vecBlock[j].MagneticPoint_Abs.push_back(megneticPoint_Abs);
            }
            if(m_vecBlock.at(j).refid == tailRefid1)
            {
                QPoint megneticPointT = QPoint((line1.tailCoordinates.rx() - m_vecBlock[j].top_left_x),(line1.tailCoordinates.ry() - m_vecBlock[j].top_left_y));//line1.tailCoordinates;
                m_vecBlock[j].MagneticPoint.push_back(megneticPointT);
                QPoint megneticPoint_AbsT = QPoint(line1.tailCoordinates.rx() ,line1.tailCoordinates.ry());
                m_vecBlock[j].MagneticPoint_Abs.push_back(megneticPoint_AbsT);
            }
        }

    }
    /************************创建节点******************************/
    for (int i = 0; i < m_vecBlock.size(); i++)
    {
        BlockInfo block = m_vecBlock.at(i);
        m_vecBlock[i].SerialNumber = i;
        createNode(block.name,block.top_left_x,block.top_left_y,block.width,block.height,block.MagneticPoint);

        //createNode();
    }
    /************************创建线***********************/


    for (int i = 0; i < m_vecline.size(); i++)
    {
        LineInfo line = m_vecline.at(i);
        Chart_Base * head;
        Chart_Base * tail;
        int mgtcHIndex;
        int mgtcTIndex;
        QPoint headPoint = line.headCoordinates;
        QPoint tailPoint = line.tailCoordinates;

        BlockInfo headBloak,tailBlock;
        for (int j = 0; j < m_vecBlock.size(); j++)
        {
            int num = m_vecBlock.at(j).SerialNumber;

            if(line.headRefid == m_vecBlock.at(j).refid)
            {

                head = chart_xml.at(num);
                headBloak =  m_vecBlock.at(j);
            }
            if(line.tailRefid == m_vecBlock.at(j).refid)
            {

                tail = chart_xml.at(num);
                tailBlock = m_vecBlock.at(j);
            }

        }
        for (int i = 0;i < headBloak.MagneticPoint_Abs.size(); i++)
        {
            if(headPoint == headBloak.MagneticPoint_Abs.at(i))
            {
                mgtcHIndex = i;
            }
        }

        for (int i = 0;i < tailBlock.MagneticPoint_Abs.size(); i++)
        {
            if(tailPoint == tailBlock.MagneticPoint_Abs.at(i))
            {
                mgtcTIndex = i;
            }
        }

        createLine(head,line.headCoordinates,mgtcHIndex,tail,line.tailCoordinates,mgtcTIndex);

    }
    //qDebug() << "哈哈哈" << endl;
    // 关联后可以关闭文件了
    file.close();
}

void FlowChart::createNode(QString name,int x,int y,int wide,int height,QVector<QPoint> vecPoint)
{
    /***********为了保证节点位置的准确性，我把边缘区域改成0了 borderWidth = 0;********/
    qDebug() << "FlowChart::createNode" << x <<" " << y << " " << wide << " " << height << endl;
    curPaintChart = new Chart_RectMy(this);
    if(curPaintChart)
    {
        mouseEventType = MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE;
    }
//    int x = 100;
//    int y = 100;

    if(curSelecChart) curSelecChart->hideMagSize();
    curSelecChart = curPaintChart;
    curPaintChart->setXY(x,y);

    curSelecChart->setNodeName(name);


    connect(curPaintChart,SIGNAL(sendThisClass(Chart_Base *, int,int)),this,SLOT(setSelecChart(Chart_Base *, int,int)));
    connect(curPaintChart,SIGNAL(setTypeChangeSize(ORIENTION)),this,SLOT(setTypeChangeSize(ORIENTION)));
    connect(curPaintChart,SIGNAL(setTypeCreateMagPoint(Chart_Base *,ORIENTION,int)),this,SLOT(setTypeCreateMagPoint(Chart_Base *,ORIENTION,int)));
    //connect(curPaintChart,SIGNAL(hideThisClass()),this,SLOT(resetSeletcChart()));
    addChart(curPaintChart);
    chart_xml.push_back(curPaintChart);
    emit sendChartStyle(curSelecChart->paintChartDrawPen,curSelecChart->paintChartFillPen);
    curPaintChart->update();
    curPaintChart->show();
    //mouseEventType = MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE;

    //int x1 = 200,y1 = 230;
    if(curSelecChart)
    {


        curPaintChart->setWidthHeight(x+wide,y+height,vecPoint);
    }

    resetPaintChartType();


    curPaintChart = nullptr;
    mouseEventType = MOUSE_EVENT_TYPE::NONE;
    setFileSetSaved(false);
}

//自定义创建线
void FlowChart::createLine()
{
    /**************创建节点1************/
    testPaintChart1 = new Chart_Rect(this);
    if(testPaintChart1)
    {
        mouseEventType = MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE;
    }
    int x_start = 100;
    int y_start = 100;

    if(curSelecChart) curSelecChart->hideMagSize();
    curSelecChart = testPaintChart1;
    testPaintChart1->setXY(x_start,y_start);

    connect(testPaintChart1,SIGNAL(sendThisClass(Chart_Base *, int,int)),this,SLOT(setSelecChart(Chart_Base *, int,int)));
    connect(testPaintChart1,SIGNAL(setTypeChangeSize(ORIENTION)),this,SLOT(setTypeChangeSize(ORIENTION)));
    connect(testPaintChart1,SIGNAL(setTypeCreateMagPoint(Chart_Base *,ORIENTION,int)),this,SLOT(setTypeCreateMagPoint(Chart_Base *,ORIENTION,int)));
    //connect(curPaintChart,SIGNAL(hideThisClass()),this,SLOT(resetSeletcChart()));
    addChart(testPaintChart1);
    emit sendChartStyle(curSelecChart->paintChartDrawPen,curSelecChart->paintChartFillPen);
    testPaintChart1->update();
    testPaintChart1->show();
    //mouseEventType = MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE;

    int x1_end = 200,y1_end = 230;
    if(testPaintChart1)
    {


        testPaintChart1->setWidthHeight(x1_end,y1_end);
    }
    resetPaintChartType();


    mouseEventType = MOUSE_EVENT_TYPE::NONE;
    setFileSetSaved(false);
    /**************创建节点2************/
    testPaintChart2 = new Chart_Rect(this);
    if(testPaintChart2)
    {
        mouseEventType = MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE;
    }
    int x_start2 = 300;
    int y_start2 = 300;

    if(curSelecChart) curSelecChart->hideMagSize();
    curSelecChart = testPaintChart2;
    testPaintChart2->setXY(x_start2,y_start2);

    connect(testPaintChart2,SIGNAL(sendThisClass(Chart_Base *, int,int)),this,SLOT(setSelecChart(Chart_Base *, int,int)));
    connect(testPaintChart2,SIGNAL(setTypeChangeSize(ORIENTION)),this,SLOT(setTypeChangeSize(ORIENTION)));
    connect(testPaintChart2,SIGNAL(setTypeCreateMagPoint(Chart_Base *,ORIENTION,int)),this,SLOT(setTypeCreateMagPoint(Chart_Base *,ORIENTION,int)));
    //connect(curPaintChart,SIGNAL(hideThisClass()),this,SLOT(resetSeletcChart()));
    addChart(testPaintChart2);
    emit sendChartStyle(curSelecChart->paintChartDrawPen,curSelecChart->paintChartFillPen);
    testPaintChart2->update();
    testPaintChart2->show();
    //mouseEventType = MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE;

    int x1_End2 = 500,y1_End2 = 400;
    if(testPaintChart2)
    {


        testPaintChart2->setWidthHeight(x1_End2,y1_End2);
    }
    resetPaintChartType();



    mouseEventType = MOUSE_EVENT_TYPE::NONE;
    setFileSetSaved(false);
    //createLine(testPaintChart1,QPoint(200,230),testPaintChart2,QPoint(300,300));

    /********************创建连线 已知存在的两个矩形 创建连线连接他们***********************/
//    PaintChartType tmp;
//    Chart_Base *cb;
//    Chart_Line *cl;
//    int id;
//    //fin.readRawData(reinterpret_cast<char*>(&tmp),sizeof(PaintChartType));
//    cb = new Chart_Line(this);
//    addLine(cb);
//    if(nullptr == (cl = dynamic_cast<Chart_Line*>(cb))) qDebug()<<"error";
//    testPaintChart1->addMagiPointStartLine(4,cl);//4是磁力点索引 从左上角0 最上面那条线中间1 右上角2...绕圈
//    cl->setStartChart(testPaintChart1);//设置起点指向的图形
//    cl->setStartPos(200,230);

//    testPaintChart2->addMagiPointEndLine(0,cl);
//    cl->setEndChart(testPaintChart2);//设置终点指向的图形
//    cl->setEndPos(300,300);

//    connect(cl,SIGNAL(sendThisClass(Chart_Base *, int,int)),this,SLOT(setSelecChart(Chart_Base *, int,int)));
//    connect(cl,SIGNAL(setTypeChangeSize(ORIENTION)),this,SLOT(setTypeChangeSize(ORIENTION)));
//    cl->applyWidthHeight();
//    cl->update();
//    cl->show();

}

void FlowChart::createLine(Chart_Base *Chart1,QPoint chart1_p,int index1,Chart_Base *Chart2,QPoint chart2_p,int index2)
{

    Chart_Base *cb;
    Chart_Line *cl;
    cb = new Chart_Line(this);
    addLine(cb);
    if(nullptr == (cl = dynamic_cast<Chart_Line*>(cb))) qDebug()<<"error";
    Chart1->addMagiPointStartLine(index1,cl);//4是磁力点索引 从左上角0 最上面那条线中间1 右上角2...绕圈
    cl->setStartChart(Chart1);//设置起点指向的图形
    cl->setStartPos(chart1_p.rx(),chart1_p.ry());

    Chart2->addMagiPointEndLine(index2,cl);
    cl->setEndChart(Chart2);//设置终点指向的图形
    cl->setEndPos(chart2_p.rx(),chart2_p.ry());

    connect(cl,SIGNAL(sendThisClass(Chart_Base *, int,int)),this,SLOT(setSelecChart(Chart_Base *, int,int)));
    connect(cl,SIGNAL(setTypeChangeSize(ORIENTION)),this,SLOT(setTypeChangeSize(ORIENTION)));
    cl->applyWidthHeight();
    cl->update();
    cl->show();
}
void FlowChart::createLine(int head_x,int head_y,int tail_x,int tail_y)
{
    curPaintChart = new Chart_Line(this);;
    if(curPaintChart)
    {
        mouseEventType = MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE;
    }

    int x = head_x;//200;
    int y = head_y;//200;
    //curPaintChart->setObjectName(QStringLiteral("tttt"));
    if(curSelecChart) curSelecChart->hideMagSize();
    curSelecChart = curPaintChart;
    curPaintChart->setXY(x,y);
    //curPaintChart->setAttribute(Qt::WA_StyledBackground,true);
    //curPaintChart->setStyleSheet(QStringLiteral("background-color: rgb(100, 100, 100);"));
    if(curPaintChart->chartType == PaintChartType::LINE)
    {
        connect(curPaintChart,SIGNAL(sendThisClass(Chart_Base *, int,int)),this,SLOT(setSelecChart(Chart_Base *, int,int)));
        connect(curPaintChart,SIGNAL(setTypeChangeSize(ORIENTION)),this,SLOT(setTypeChangeSize(ORIENTION)));
        addLine(curPaintChart);
        Chart_Line *cl = dynamic_cast<Chart_Line*>(curSelecChart);
        emit sendLineStyle(cl->paintChartDrawPen,cl->getStartLineHeadType(),cl->getEndLineHeadType());
    }
    curPaintChart->update();
    curPaintChart->show();
    //mouseEventType = MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE;

    int x1 = tail_x,y1 = tail_y;
    if(curSelecChart)
    {
        if(curSelecChart->chartType == PaintChartType::LINE)
        {
            for(auto it = charts.begin();it!=charts.end();it++)
            {
                if((*it)->autoSetMagi(x,y,magPointIndex))
                {
                    lineSelectChart = *it;
                    break;
                }else{
                    lineSelectChart = nullptr;
                }
            }
        }

        curPaintChart->setWidthHeight(x1,y1);
    }

    //curPaintChart->updatePath();
    resetPaintChartType();
    if(curSelecChart->chartType == PaintChartType::LINE)
    {
        Chart_Line *cl = reinterpret_cast<Chart_Line *>(curSelecChart);

        if(lineSelectChart)
        {
            lineSelectChart->addMagiPointEndLine(magPointIndex,curSelecChart);
            lineSelectChart->hideMagOnly();
            cl->setEndChart(lineSelectChart);
            cl->setEndMagIndex(magPointIndex);
            cl->setEndDirect(lineSelectChart->getMagiPointDirect(magPointIndex));
            cl->update();
            lineSelectChart = nullptr;
        }
        else{
            cl->resetEndChart();
        }

    }


    curPaintChart = nullptr;
    mouseEventType = MOUSE_EVENT_TYPE::NONE;
    setFileSetSaved(false);


}
//自定义创建线
void FlowChart::createLine(QPoint head,QPoint tail)
{
//    curPaintChart = new Chart_Line(this);
//    if(curPaintChart)
//    {
//        mouseEventType = MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE;
//    }


//    //curPaintChart->setObjectName(QStringLiteral("tttt"));
//    if(curSelecChart) curSelecChart->hideMagSize();
//    curSelecChart = curPaintChart;
//    curPaintChart->setXY(head.rx(),head.ry());
//    //curPaintChart->setAttribute(Qt::WA_StyledBackground,true);
//    //curPaintChart->setStyleSheet(QStringLiteral("background-color: rgb(100, 100, 100);"));
//    if(curPaintChart->chartType == PaintChartType::LINE)
//    {
//        connect(curPaintChart,SIGNAL(sendThisClass(Chart_Base *, int,int)),this,SLOT(setSelecChart(Chart_Base *, int,int)));
//        connect(curPaintChart,SIGNAL(setTypeChangeSize(ORIENTION)),this,SLOT(setTypeChangeSize(ORIENTION)));
//        addLine(curPaintChart);
//        Chart_Line *cl = dynamic_cast<Chart_Line*>(curSelecChart);
//        emit sendLineStyle(cl->paintChartDrawPen,cl->getStartLineHeadType(),cl->getEndLineHeadType());
//    }
//    curPaintChart->update();
//    curPaintChart->show();
//    //mouseEventType = MOUSE_EVENT_TYPE::CREATING_CNANGE_SIZE;

//    if(curSelecChart)
//    {
//        if(curSelecChart->chartType == PaintChartType::LINE)
//        {
//            for(auto it = charts.begin();it!=charts.end();it++)
//            {
//                if((*it)->autoSetMagi(head.rx(),head.ry(),magPointIndex))
//                {
//                    lineSelectChart = *it;
//                    break;
//                }else{
//                    lineSelectChart = nullptr;
//                }
//            }
//        }

//        curPaintChart->setWidthHeight(tail.rx(),tail.ry());
//    }

//    //curPaintChart->updatePath();
//    resetPaintChartType();
//    if(curSelecChart->chartType == PaintChartType::LINE)
//    {
//        Chart_Line *cl = reinterpret_cast<Chart_Line *>(curSelecChart);

//        if(lineSelectChart)
//        {
//            lineSelectChart->addMagiPointEndLine(magPointIndex,curSelecChart);
//            lineSelectChart->hideMagOnly();
//            cl->setEndChart(lineSelectChart);
//            cl->setEndMagIndex(magPointIndex);
//            cl->setEndDirect(lineSelectChart->getMagiPointDirect(magPointIndex));
//            cl->update();
//            lineSelectChart = nullptr;
//        }
//        else{
//            cl->resetEndChart();
//        }

//    }


//    curPaintChart = nullptr;
//    mouseEventType = MOUSE_EVENT_TYPE::NONE;
//    setFileSetSaved(false);
}
