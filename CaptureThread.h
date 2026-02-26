#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H

#include <QThread>
#include <QImage>
#include "WindowCapture.h"

class CaptureThread : public QThread
{
    Q_OBJECT
public:
    explicit CaptureThread(QObject *parent = nullptr);
    ~CaptureThread() override;

    // 设置捕获窗口句柄
    void setWindowHandle(HWND hwnd);
    // 设置手牌区域参数
    void setHandROIRatio(float x, float y, float w, float h);
    // 停止线程
    void stop();
    // 设置是否显示放大的手牌区域
    void setShowHandZoom(bool zoom);

signals:
    // 捕获到新画面的信号（主线程更新界面用）
    void frameCaptured(QImage frame, QImage handFrame);
    // 捕获错误信号
    void captureError(QString msg);

protected:
    void run() override;

private:
    bool m_isRunning = false;
    HWND m_hwnd = NULL;
    WindowCapture m_capture;
    // 手牌区域参数
    float m_xRatio = 0.1f;
    float m_yRatio = 0.8f;
    float m_widthRatio = 0.8f;
    float m_heightRatio = 0.2f;
    bool m_showHandZoom = false;

    // OpenCV Mat转QImage
    QImage matToQImage(const cv::Mat& mat);
};

#endif // CAPTURETHREAD_H