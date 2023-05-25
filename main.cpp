#include <opencv2/opencv.hpp>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ncurses.h>
#include <unistd.h>
#include <iostream>
#include <vector>

class AsciiAnim
{
    public:
        AsciiAnim();
        AsciiAnim(int w, int h);
        ~AsciiAnim();
        void Init(int w, int h);
        int Insert(char *buf, int size);
        void GetWH(int& w, int& h) const
        {
            w = this->_w;
            h = this->_h;
        };
        const char* LoopGetFrame();
    private:
        int _framecount;
        int _w;
        int _h;
        int _curframe;
        std::vector<char*> _framebuf;
};

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

int AsciiAnim::Insert(char* buf, int size)
{
    if (!buf) {
        return -1;
    }
    int realsize = std::min(size, (this->_w * this->_h));
    char* innerbuf = new char[realsize];
    memcpy(innerbuf, buf, realsize * sizeof(char));
    this->_framebuf.push_back(innerbuf);
    this->_framecount++;
    return 0;
}
const char* AsciiAnim::LoopGetFrame()
{
    const char* ret = this->_framebuf.at(this->_curframe);
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

int Image2Ascii(cv::Mat& frame, int w, int h, AsciiAnim& anim)
{
    cv::Mat gray, resized;
    cv::cvtColor(frame, gray, cv::COLOR_RGB2GRAY);
    cv::resize(gray, resized, cv::Size(w, h));
    char* buf = new char[w*h];
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            uint8_t c = resized.data[y * w + x];
            buf[y * w + x] = Gray2Ascii(c);
        }
    }
    anim.Insert(buf, w*h);
    delete [] buf;
    return 0;
}

int ImageFile2Ascii(const std::string& path, int w, int h, AsciiAnim& anim)
{
    cv::Mat frame;
    frame = cv::imread(path, cv::COLOR_RGB2GRAY);
    Image2Ascii(frame, w, h, anim);
    return 0;
}

int VideoFile2Ascii(const std::string& path, int w, int h, AsciiAnim& anim)
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
        Image2Ascii(frame, w, h, anim);
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

    const char* buf = NULL;
    mainscr = initscr();
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
        ImageFile2Ascii(argv[2], maxw, maxh, anim);
        input_type = TYPE_IMAGE;
    } else {
        VideoFile2Ascii(argv[2], maxw, maxh, anim);
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
                mvaddch(y, x, buf[y * maxw + x]);
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
