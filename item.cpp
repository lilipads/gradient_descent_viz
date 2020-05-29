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


LabeledItem::~LabeledItem(){
    if (m_label != nullptr){
        // since QCustom3DLabel is a QObject, it will automatically take care
        // of deleting its child pointers
        m_graph->releaseCustomItem(m_label);
    }
}

void LabeledItem::setLabel(const QString &text){
    // the graph somehow doesn't update the text on its own
    // so we need this roundabout way of removing the label
    // and reinitializing it
    if (m_label != nullptr){
        m_graph->removeCustomItem(m_label);
    }

    m_label = new QCustom3DLabel;
    m_label->setFacingCamera(true);
    m_label->setScaling(QVector3D(1., 1., 1.));
    m_label->setVisible(label_visibility && this->isVisible());

    m_label->setText(text);
    label_visibility = true;
    m_label->setVisible(label_visibility && this->isVisible());
    m_label->setPosition(position() + label_offset());

    m_graph->addCustomItem(m_label);
}


void LabeledItem::setVisible(bool visible){
    QCustom3DItem::setVisible(visible);
    if (m_label != nullptr)
        m_label->setVisible(label_visibility && isVisible());
}


void LabeledItem::setLabelVisibility(bool visible){
    label_visibility = visible;
    if (m_label != nullptr)
        m_label->setVisible(visible && this->isVisible());
}


void LabeledItem::setPosition(const QVector3D & position){
    QCustom3DItem::setPosition(position);
    if (m_label != nullptr)
        m_label->setPosition(position + label_offset());
}


Ball::Ball(Q3DSurface* graph, QColor color){
    setScaling(QVector3D(0.01f, 0.01f, 0.01f));
    setMeshFile(QStringLiteral(":/mesh/largesphere.obj"));
    setColor(color);
    addToGraph(graph);
}


Arrow::Arrow(Q3DSurface* graph) : LabeledItem(graph){
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
    float magnitude_in_unit_arrow = magnitude * kUnitItemPerGraph / unitPlotPerGraph * kItemScale;
    setScaling(QVector3D(0.1, 0.1 * magnitude_in_unit_arrow, 0.1));
}


Square::Square(Q3DSurface* graph) : LabeledItem(){
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

