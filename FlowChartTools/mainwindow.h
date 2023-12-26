#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include "comdef.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
public slots:
    void setFillStyle(QPen &qp, QBrush &qb);
    void setLineStyle(QPen &qp, LINE_HEAD_TYPE &startLineHeadType, LINE_HEAD_TYPE &endLineHeadTypeb);
    void disableStylePanel();
    void sendSelChartLineColor();
    void sendSelChartFillColor();
    void changeWindowTitle(const QString &);

private slots:
    void on_flowChart_widget_customContextMenuRequested(const QPoint &pos);
};

#endif // MAINWINDOW_H
