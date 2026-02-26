#include "CaptureThread.h"
#include <opencv2/opencv.hpp>

CaptureThread::CaptureThread(QObject *parent)
    : QThread(parent)
{
}

CaptureThread::~CaptureThread()
{
    stop();
}

void CaptureThread::setWindowHandle(HWND hwnd)
{
    m_hwnd = hwnd;
    m_capture.setWindowHandle(hwnd);
}

void CaptureThread::setHandROIRatio(float x, float y, float w, float h)
{
    m_xRatio = x;
    m_yRatio = y;
    m_widthRatio = w;
    m_heightRatio = h;
}

void CaptureThread::stop()
{
    m_isRunning = false;
    wait();
}

void CaptureThread::setShowHandZoom(bool zoom)
{
    m_showHandZoom = zoom;
}

QImage CaptureThread::matToQImage(const cv::Mat& mat)
{
    if (mat.empty()) return QImage();
    cv::Mat rgbMat;
    cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
    return QImage(rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step, QImage::Format_RGB888).copy();
}

void CaptureThread::run()
{
    m_isRunning = true;
    while (m_isRunning)
    {
        if (m_hwnd == NULL || !IsWindow(m_hwnd))
        {
            emit captureError("窗口句柄无效，请重新选择游戏窗口");
            msleep(100);
            continue;
        }

        // 捕获窗口画面
        cv::Mat windowMat;
        if (!m_capture.captureWindow(windowMat))
        {
            msleep(100);
            continue;
        }

        // 定位手牌区域
        cv::Rect handROI = m_capture.getHandCardROI(windowMat, m_xRatio, m_yRatio, m_widthRatio, m_heightRatio);
        cv::Mat handMat = windowMat(handROI).clone();

        // 绘制手牌区域框
        cv::rectangle(windowMat, handROI, cv::Scalar(0, 255, 0), 2);
        cv::putText(windowMat, "Hand Card Area", cv::Point(handROI.x, handROI.y - 10),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);

        // 转换为QImage
        QImage windowImg = matToQImage(windowMat);
        QImage handImg = matToQImage(handMat);

        // 发送信号到主线程
        emit frameCaptured(m_showHandZoom ? handImg : windowImg, handImg);

        // 控制帧率，降低CPU占用
        msleep(30);
    }
}