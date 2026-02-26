#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include "CaptureThread.h"
#include "ZhangXuanCalculator.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 主界面功能
    void on_addCardBtn_clicked();
    void on_delCardBtn_clicked();
    void on_clearCardBtn_clicked();
    void on_calcBtn_clicked();

    // 窗口捕获功能
    void on_refreshWindowBtn_clicked();
    void on_startCaptureBtn_clicked();
    void on_stopCaptureBtn_clicked();
    void on_zoomHandCheckBox_stateChanged(int arg1);
    void on_roiXSlider_valueChanged(int value);
    void on_roiYSlider_valueChanged(int value);
    void on_roiWSlider_valueChanged(int value);
    void on_roiHSlider_valueChanged(int value);

    // 卡牌库功能
    void on_searchCardEdit_textChanged(const QString &arg1);
    void on_modifyProfitBtn_clicked();
    void on_addCustomCardBtn_clicked();
    void on_delCardLibBtn_clicked();

    // 捕获线程槽函数
    void onFrameCaptured(QImage frame, QImage handFrame);
    void onCaptureError(QString msg);

private:
    Ui::MainWindow *ui;
    CaptureThread *m_captureThread;
    ZhangXuanCalculator m_calculator;

    // 初始化函数
    void initUI();
    void initCardData();
    void initWindowList();
    void loadCardLibraryTable(const QString& filter = "");

    // 工具函数
    void updateROILabel();
    void showResult(const CalculateResult& result);
};

#endif // MAINWINDOW_H