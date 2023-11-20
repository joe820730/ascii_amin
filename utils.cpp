#include <opencv2/opencv.hpp>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include "asciiamin.h"
#include "utils.h"

char Gray2Ascii(uint8_t c)
{
    // http://paulbourke.net/dataformats/asciiart
    const char* table = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
    int idx = strlen(table) * c / 256;
    return table[idx];
}

uint8_t Rgb2Ascii(int r, int g, int b)
{
    uint8_t color = 0;
    uint32_t r_scaled = (r * 24);
    uint32_t g_scaled = (g * 24);
    uint32_t b_scaled = (b * 24);
    if (r_scaled == g_scaled && g_scaled == b_scaled) {
        color = (r_scaled >> 8) + 232;
    } else {
        r_scaled = (r_scaled >> 2) >> 8;
        g_scaled = (g_scaled >> 2) >> 8;
        b_scaled = (b_scaled >> 2) >> 8;
        color = 16 + (r_scaled * 36 + g_scaled * 6 + b_scaled);
    }
    return color;
}

int Image2Ascii(cv::Mat& frame, int w, int h, AsciiAnim& anim, bool color_support)
{
    cv::Mat gray, resized;
    if (color_support) {
        cv::resize(frame, resized, cv::Size(w, h));
    } else {
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::resize(gray, resized, cv::Size(w, h));
    }
    uint16_t* buf = new uint16_t[w*h];
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (color_support) {
                cv::Vec3b & color_ = resized.at<cv::Vec3b>(y,x);
                uint8_t color = Rgb2Ascii(color_[2], color_[1], color_[0]);
                buf[y * w + x] = ' ' | (color << 8);
            } else {
                uint8_t c = resized.data[y * w + x];
                buf[y * w + x] = Gray2Ascii(c);
            }
        }
    }
    anim.Insert(buf, w*h);
    delete [] buf;
    return 0;
}

int ImageFile2Ascii(const std::string& path, int w, int h, AsciiAnim& anim, bool color_support)
{
    cv::Mat frame;
    frame = cv::imread(path, cv::IMREAD_COLOR);
    Image2Ascii(frame, w, h, anim, color_support);
    return 0;
}

int VideoFile2Ascii(const std::string& path, int w, int h, AsciiAnim& anim, bool color_support)
{
    cv::VideoCapture vid(path);
    if (!vid.isOpened()) {
        return -1;
    }
    cv::Mat frame;
    while (true) {
        if (!vid.read(frame)) {
            break;
        }
        Image2Ascii(frame, w, h, anim, color_support);
    }
    vid.release();
    return 0;
}
