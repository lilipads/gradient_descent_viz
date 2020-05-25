#ifndef SURFACE_H
#define SURFACE_H

#include <QtDataVisualization/Q3DSurface>

using namespace QtDataVisualization;

class Surface : public Q3DSurface{
public:
    float sampleMinX = -8.0f;
    float sampleMaxX = 8.0f;
    float sampleMinZ = -8.0f;
    float sampleMaxZ = 8.0;
};


#endif // SURFACE_H
