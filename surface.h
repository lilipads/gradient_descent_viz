#ifndef SURFACE_H
#define SURFACE_H

#include <QtDataVisualization/Q3DSurface>

using namespace QtDataVisualization;

class Surface : public Q3DSurface{
public:
    float minX = -1.0f;
    float maxX = 1.0f;
    float minZ = -1.0f;
    float maxZ = 1.0;
};


#endif // SURFACE_H
