#ifndef ZBUFFER_INCLUDED
#define ZBUFFER_INCLUDED

#include <limits>
#include <vector>

class ZBuffer : public std::vector<std::vector<double>> {
public:
    ZBuffer(unsigned int width, unsigned int height)
        : std::vector<std::vector<double>>(width, std::vector<double>(height, std::numeric_limits<double>::infinity())) {}
};

#endif
