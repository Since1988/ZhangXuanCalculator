#ifndef WINDOWCAPTURE_H
#define WINDOWCAPTURE_H

#include <opencv2/opencv.hpp>
#include <windows.h>
#include <string>
#include <vector>

class WindowCapture {
public:
    WindowCapture() = default;
    ~WindowCapture();

    // 查找三国杀游戏窗口（支持模糊匹配标题）
    bool findGameWindow(const std::string& windowKeyword = "三国杀");
    // 手动指定窗口句柄
    void setWindowHandle(HWND hwnd);
    // 获取当前捕获的窗口句柄
    HWND getWindowHandle() const;
    // 获取窗口标题列表（用于调试，查看所有窗口标题）
    static std::vector<std::pair<HWND, std::string>> getAllWindowTitles();

    // 核心：捕获窗口画面，返回OpenCV Mat格式
    bool captureWindow(cv::Mat& outImg);

    // 自动定位手牌区域（基于窗口比例，可自定义调整）
    cv::Rect getHandCardROI(const cv::Mat& windowImg, 
                             float xRatio = 0.1f,   // 手牌区域左边界占窗口宽度比例
                             float yRatio = 0.8f,   // 手牌区域上边界占窗口高度比例
                             float widthRatio = 0.8f, // 手牌区域宽度占窗口宽度比例
                             float heightRatio = 0.2f);// 手牌区域高度占窗口高度比例
private:
    HWND m_hwnd = NULL;
    HDC m_hWindowDC = NULL;
    HDC m_hMemoryDC = NULL;
    HBITMAP m_hBitmap = NULL;
    BYTE* m_pBitmapData = NULL;
    int m_windowWidth = 0;
    int m_windowHeight = 0;

    // 释放资源
    void releaseResources();
    // 窗口枚举回调
    static BOOL CALLBACK enumWindowProc(HWND hwnd, LPARAM lParam);
};

#endif // WINDOWCAPTURE_H