#include "asciiamin.h"

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
