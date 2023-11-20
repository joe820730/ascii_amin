#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED
#include "asciianim.h"
#include <string>

extern int VideoFile2Ascii(const std::string& path, int w, int h, AsciiAnim& anim, bool color_support);
extern int ImageFile2Ascii(const std::string& path, int w, int h, AsciiAnim& anim, bool color_support);

#endif // UTILS_H_INCLUDED
