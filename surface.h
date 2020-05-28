#ifndef SURFACE_H
#define SURFACE_H

#include <QtDataVisualization/Q3DSurface>

using namespace QtDataVisualization;

class Surface : public Q3DSurface{
public:
    float minX = -8.0f;
    float maxX = 8.0f;
    float minZ = -8.0f;
    float maxZ = 8.0;
    ~Surface(){qDebug() << "destructor for surface\n";}
};


#endif // SURFACE_H
