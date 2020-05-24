#include "item.h"

Item::Item()
{

}

Ball::Ball(QColor color){
    setScaling(QVector3D(0.01f, 0.01f, 0.01f));
    setMeshFile(QStringLiteral(":/mesh/largesphere.obj"));
    QImage pointColor = QImage(2, 2, QImage::Format_ARGB32);
    pointColor.fill(color);
    setTextureImage(pointColor);
}

Arrow::Arrow(QVector3D direction){
    // negative 1 because oddly, the default xyz axis
    // in Q3DSurface has Z axis flipped
    direction.setZ(-direction.z());
    setMeshFile(QStringLiteral(":/mesh/narrowarrow.obj"));
    QImage pointColor = QImage(2, 2, QImage::Format_RGB32);
    pointColor.fill(Qt::black);
    setTextureImage(pointColor);
    QQuaternion rotation = QQuaternion::rotationTo(
                QVector3D(0, 1, 0), direction);
    setRotation(rotation);
    setScaling(QVector3D(0.1f, 0.5f, 0.1f));
    setPosition(QVector3D(0, 5, 0));
}
