#include <math.h>

#include "item.h"

void Item::setColor(QColor color){
    QImage pointColor = QImage(2, 2, QImage::Format_ARGB32);
    pointColor.fill(color);
    setTextureImage(pointColor);
}


void Item::addToGraph(Q3DSurface *graph){
    if (m_graph == nullptr){
        m_graph = graph;
        m_graph->addCustomItem(this);
    }
}


QVector3D Item::plotScalingVector(){
    return QVector3D(m_graph->axisX()->max() - m_graph->axisX()->min(), 0,
                     m_graph->axisZ()->max() - m_graph->axisZ()->min());
}


Ball::Ball(Q3DSurface* graph, QColor color, double (*_f) (double, double))
    : f(_f)
{
    setScaling(QVector3D(0.01f, 0.01f, 0.01f));
    setMeshFile(QStringLiteral(":/mesh/largesphere.obj"));
    setColor(color);
    addToGraph(graph);
}


void Ball::setPositionOnSurface(double x, double z){
    float yOffset =  (m_graph->axisY()->max() - m_graph->axisY()->min()) /
            kBallRadiusPerGraph;
    setPosition(QVector3D(x, f(x, z) + yOffset, z));
}

Arrow::Arrow(Q3DSurface* graph) : Item(graph){
    setMeshFile(QStringLiteral(":/mesh/narrowarrow.obj"));
    setMagnitude(0);
    setColor(Qt::black);
}


Arrow::Arrow(Q3DSurface* graph, QVector3D vector): Arrow(graph) {
    setVector(vector);
}


Arrow::Arrow(Q3DSurface* graph, QVector3D vector, QColor color) {
    m_graph = graph;
    setMeshFile(QStringLiteral(":/mesh/narrowarrow.obj"));
    setMagnitude(0);
    setColor(color);
    setVector(vector);
    m_graph->addCustomItem(this);
}


void Arrow::setVector(QVector3D vector){
    /* draw an arrow representing the vector (direction and magnitude) */

    direction = vector.normalized();
    // z times -1 because oddly, the default xyz axis in Q3DSurface has Z axis flipped
    QQuaternion rotation = QQuaternion::rotationTo(
                QVector3D(0, 1, 0), direction * QVector3D(1, 1, -1));
    setRotation(rotation);
    setMagnitude(vector.length());
}

void Arrow::setMagnitude(const float &magnitude){
    // if magnitude is negative, arrow extends in the other direction
    float unitPlotPerGraph = (direction * plotScalingVector()).length();

    m_magnitude = magnitude;

    float magnitude_in_unit_arrow = magnitude * kUnitItemPerGraph /
            unitPlotPerGraph * kItemScale;
    // if the arrow is really small, make sure its tip still extends outside of the ball
    float min_magnitude = kBallRadiusPerGraph / (0.1 * kUnitItemPerGraph) * 1.2;
    if (abs(magnitude_in_unit_arrow) < min_magnitude)
        magnitude_in_unit_arrow = min_magnitude * magnitude_in_unit_arrow / abs(magnitude_in_unit_arrow);
    setScaling(QVector3D(0.1, 0.1 * magnitude_in_unit_arrow, 0.1));
}


Square::Square(Q3DSurface* graph) : Item(){
    setMeshFile(QStringLiteral(":/mesh/plane.obj"));
    setScaling(QVector3D(0.1, 0.1, 0.1));
    QColor color = Qt::white;
    color.setAlpha(150);
    setColor(color);
    setRotationAxisAndAngle(QVector3D(0, 0, 1), 90);
    addToGraph(graph);
}

void Square::setArea(const float &area){
    float unitPlotPerGraph = plotScalingVector().length();
    float scale = sqrt(area) * kUnitItemPerGraph / unitPlotPerGraph * kItemScale;
    setScaling(QVector3D(scale, 1, scale) * 0.1);
    m_area = area;
}

