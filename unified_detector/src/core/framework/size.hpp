#ifndef __CORE_SIZE__
#define __CORE_SIZE__

#include <string>
namespace unified_detector{
class Size {
public:
    int width;
    int height;

    inline bool isEmpty() {
        return width < 1 || height < 1;
    }

    Size() : width(0), height(0) {}

    Size(int _width, int _height) : width(_width), height(_height) {}
};
} // unified_detector
#endif //__CORE_SIZE__
