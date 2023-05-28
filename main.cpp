#include <opencv2/opencv.hpp>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ncurses.h>
#include <unistd.h>
#include <iostream>
#include <vector>

typedef void (*PainterFn)(int, int, uint16_t);
class AsciiAnim
{
    public:
        AsciiAnim();
        AsciiAnim(int w, int h);
        ~AsciiAnim();
        void Init(int w, int h);
        int Insert(uint16_t *buf, int size);
        void GetWH(int& w, int& h) const
        {
            w = this->_w;
            h = this->_h;
        };
        const uint16_t* LoopGetFrame();
    private:
        int _framecount;
        int _w;
        int _h;
        int _curframe;
        std::vector<uint16_t*> _framebuf;
};

void Init_colorpair()
{
    for (int i = 0; i < COLORS; i++) {
        init_pair(i, i, i);
    }
}

void PaintGray(int x, int y, uint16_t buf)
{
    mvaddch(y, x, (uint8_t)(buf & 0xff));
}

void PaintColor(int x, int y, uint16_t buf)
{
    uint8_t color = (buf & 0xff00) >> 8;
    attron(COLOR_PAIR(color));
    mvaddch(y, x, (uint8_t)(buf & 0xff));
    attroff(COLOR_PAIR(color));
}

AsciiAnim::AsciiAnim()
{
    this->_w = 0;
    this->_h = 0;
    this->_framecount = 0;
    this->_curframe = 0;
    this->_framebuf.reserve(32);
}

AsciiAnim::AsciiAnim(int w, int h)
{
    this->_framecount = 0;
    this->_curframe = 0;
    this->_framebuf.reserve(32);
    this->Init(w, h);
}

AsciiAnim::~AsciiAnim()
{
    for (size_t i = 0, l = this->_framebuf.size(); i < l; i++) {
        free(this->_framebuf.at(i));
    }
}

void AsciiAnim::Init(int w, int h)
{
    this->_w = w;
    this->_h = h;
}

int AsciiAnim::Insert(uint16_t* buf, int size)
{
    if (!buf) {
        return -1;
    }
    int realsize = std::min(size, (this->_w * this->_h));
    uint16_t* innerbuf = new uint16_t[realsize];
    memcpy(innerbuf, buf, realsize * sizeof(uint16_t));
    this->_framebuf.push_back(innerbuf);
    this->_framecount++;
    return 0;
}
const uint16_t* AsciiAnim::LoopGetFrame()
{
    const uint16_t* ret = this->_framebuf.at(this->_curframe);
    this->_curframe++;
    if (this->_curframe >= this->_framecount) {
        this->_curframe = 0;
    }
    return ret;
}

char Gray2Ascii(uint8_t c)
{
    // http://paulbourke.net/dataformats/asciiart
    const char* table = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
    int idx = ((double)c / 255.0) * sizeof(table);
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

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: %s [option] [path/to/video/file]\nOption:\n\t-I: Image\n\t-V: Video\n", argv[0]);
        return 0;
    }
    enum {
        TYPE_IMAGE,
        TYPE_VIDEO,
        TYPE_ALL
    } input_type;
    WINDOW *mainscr = NULL;
    int i, j;
    bool loop = true;
    int maxh, maxw;
    AsciiAnim anim;

    bool color_support = false;
    PainterFn painterfn = PaintGray;

    const uint16_t* buf = NULL;
    mainscr = initscr();
    if (has_colors()) {
        start_color();
        if (COLORS == 256) {
            Init_colorpair();
            color_support = true;
            painterfn = PaintColor;
        }
    }
    if (!mainscr) {
        printf("Error, exit.\n");
    }
    getmaxyx(mainscr, maxh, maxw);
    curs_set(0);
    noecho();
    keypad(stdscr, TRUE);
    anim.Init(maxw, maxh);

    printw("Loading file %s ...\n", argv[2]);
    refresh();
    if (!strncmp(argv[1], "-I", 2)) {
        ImageFile2Ascii(argv[2], maxw, maxh, anim, color_support);
        input_type = TYPE_IMAGE;
    } else {
        VideoFile2Ascii(argv[2], maxw, maxh, anim, color_support);
        input_type = TYPE_VIDEO;
    }

    while (loop) {
        buf = anim.LoopGetFrame();
        switch (input_type) {
            case TYPE_IMAGE:
                clear();
                break;
            case TYPE_VIDEO:
                timeout(33);
                break;
            default:
                break;
        }
        for (int y = 0; y < maxh; y++) {
            for (int x = 0; x < maxw; x++) {
                painterfn(x, y, buf[y * maxw + x]);
            }
        }
        refresh();
        switch (getch()){
            case 'q':
                loop = false;
                break;
        }
    }
    endwin();
    return 0;
}
