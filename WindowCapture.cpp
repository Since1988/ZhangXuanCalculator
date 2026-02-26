#include "WindowCapture.h"
#include <iostream>
#include <algorithm>

WindowCapture::~WindowCapture() {
    releaseResources();
}

void WindowCapture::releaseResources() {
    if (m_hBitmap) DeleteObject(m_hBitmap);
    if (m_hMemoryDC) DeleteDC(m_hMemoryDC);
    if (m_hWindowDC) ReleaseDC(m_hwnd, m_hWindowDC);
    m_hBitmap = NULL;
    m_hMemoryDC = NULL;
    m_hWindowDC = NULL;
    m_pBitmapData = NULL;
    m_windowWidth = 0;
    m_windowHeight = 0;
}

// 窗口枚举回调
BOOL CALLBACK WindowCapture::enumWindowProc(HWND hwnd, LPARAM lParam) {
    auto& windowList = *reinterpret_cast<std::vector<std::pair<HWND, std::string>>*>(lParam);
    if (IsWindowVisible(hwnd)) {
        char title[256] = { 0 };
        GetWindowTextA(hwnd, title, sizeof(title));
        if (strlen(title) > 0) {
            windowList.emplace_back(hwnd, title);
        }
    }
    return TRUE;
}

// 获取所有窗口标题
std::vector<std::pair<HWND, std::string>> WindowCapture::getAllWindowTitles() {
    std::vector<std::pair<HWND, std::string>> windowList;
    EnumWindows(enumWindowProc, reinterpret_cast<LPARAM>(&windowList));
    return windowList;
}

// 查找游戏窗口
bool WindowCapture::findGameWindow(const std::string& windowKeyword) {
    auto windowList = getAllWindowTitles();
    std::string lowerKeyword = windowKeyword;
    std::transform(lowerKeyword.begin(), lowerKeyword.end(), lowerKeyword.begin(), ::tolower);

    for (auto& pair : windowList) {
        std::string lowerTitle = pair.second;
        std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::tolower);
        if (lowerTitle.find(lowerKeyword) != std::string::npos) {
            m_hwnd = pair.first;
            std::cout << "找到游戏窗口: " << pair.second << " 句柄: " << m_hwnd << std::endl;
            return true;
        }
    }

    std::cout << "未找到包含关键词[" << windowKeyword << "]的窗口，请手动指定窗口标题或句柄" << std::endl;
    return false;
}

void WindowCapture::setWindowHandle(HWND hwnd) {
    if (m_hwnd == hwnd) return;
    releaseResources();
    m_hwnd = hwnd;
}

HWND WindowCapture::getWindowHandle() const {
    return m_hwnd;
}

// 核心：捕获窗口画面
bool WindowCapture::captureWindow(cv::Mat& outImg) {
    if (!IsWindow(m_hwnd)) {
        std::cout << "窗口句柄无效！" << std::endl;
        return false;
    }

    // 获取窗口尺寸
    RECT windowRect;
    if (!GetClientRect(m_hwnd, &windowRect)) {
        std::cout << "获取窗口尺寸失败！" << std::endl;
        return false;
    }
    int width = windowRect.right - windowRect.left;
    int height = windowRect.bottom - windowRect.top;
    if (width <= 0 || height <= 0) {
        std::cout << "窗口已最小化，无法捕获！" << std::endl;
        return false;
    }

    // 资源重置（窗口尺寸变化时）
    if (width != m_windowWidth || height != m_windowHeight) {
        releaseResources();
        m_windowWidth = width;
        m_windowHeight = height;

        // 创建DC和位图
        m_hWindowDC = GetDC(m_hwnd);
        m_hMemoryDC = CreateCompatibleDC(m_hWindowDC);

        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = m_windowWidth;
        bmi.bmiHeader.biHeight = -m_windowHeight; // 顶部朝下，和OpenCV坐标系一致
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 24;
        bmi.bmiHeader.biCompression = BI_RGB;

        m_hBitmap = CreateDIBSection(m_hWindowDC, &bmi, DIB_RGB_COLORS, (void**)&m_pBitmapData, NULL, 0);
        SelectObject(m_hMemoryDC, m_hBitmap);
    }

    // 捕获窗口画面（PrintWindow支持后台窗口捕获，无需前台置顶）
    if (!PrintWindow(m_hwnd, m_hMemoryDC, PW_CLIENTONLY)) {
        std::cout << "窗口捕获失败！" << std::endl;
        return false;
    }

    // 转换为OpenCV Mat格式（BGR通道，和OpenCV默认一致）
    outImg = cv::Mat(m_windowHeight, m_windowWidth, CV_8UC3, m_pBitmapData).clone();
    return true;
}

// 自动定位手牌区域
cv::Rect WindowCapture::getHandCardROI(const cv::Mat& windowImg, float xRatio, float yRatio, float widthRatio, float heightRatio) {
    int imgW = windowImg.cols;
    int imgH = windowImg.rows;
    int x = static_cast<int>(imgW * xRatio);
    int y = static_cast<int>(imgH * yRatio);
    int w = static_cast<int>(imgW * widthRatio);
    int h = static_cast<int>(imgH * heightRatio);

    // 边界校验
    x = std::max(0, x);
    y = std::max(0, y);
    w = std::min(imgW - x, w);
    h = std::min(imgH - y, h);
    return cv::Rect(x, y, w, h);
}