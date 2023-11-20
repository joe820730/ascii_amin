#ifndef ASCIIAMIN_H_INCLUDED
#define ASCIIAMIN_H_INCLUDED
#include <stdint.h>
#include <vector>

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
#endif // ASCIIAMIN_H_INCLUDED
