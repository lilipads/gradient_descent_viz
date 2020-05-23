#include "item.h"

Item::Item()
{

}

Ball::Ball(QColor color){
    setScaling(QVector3D(0.01f, 0.01f, 0.01f));
    setMeshFile(QStringLiteral(":/mesh/largesphere.obj"));
    QImage pointColor = QImage(2, 2, QImage::Format_RGB32);
    pointColor.fill(color);
    setTextureImage(pointColor);
}
