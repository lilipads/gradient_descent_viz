#include <math.h>

#include "item.h"

void Item::setColor(QColor color){
    QImage pointColor = QImage(2, 2, QImage::Format_ARGB32);
    pointColor.fill(color);
    setTextureImage(pointColor);
}


void Item::addToGraph(Surface *graph){
    if (m_graph == nullptr){
        m_graph = graph;
        m_graph->addCustomItem(this);
    }
}


void LabeledItem::setLabel(const QString &text){
    // the graph somehow doesn't update the text on its own
    // so we need this roundabout way of removing the label
    // and reinitializing it
    if (m_label != nullptr){
        m_graph->removeCustomItem(m_label);
        m_label = nullptr;
    }

    m_label = new QCustom3DLabel;
    m_label->setFacingCamera(true);
    m_label->setScaling(QVector3D(1., 1., 1.));
    m_label->setVisible(label_visibility && this->isVisible());

    m_label->setText(text);
    label_visibility = true;
    m_label->setVisible(label_visibility && this->isVisible());

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
        m_label->setPosition(position + kLabelOffset);
}


Ball::Ball(Surface* graph, QColor color){
    setScaling(QVector3D(0.01f, 0.01f, 0.01f));
    setMeshFile(QStringLiteral(":/mesh/largesphere.obj"));
    setColor(color);
    addToGraph(graph);
}


Arrow::Arrow(Surface* graph) : LabeledItem(graph){
    setMeshFile(QStringLiteral(":/mesh/narrowarrow.obj"));
    setMagnitude(0);
    setColor(Qt::black);
}


Arrow::Arrow(Surface* graph, QVector3D vector): Arrow(graph) {
    setVector(vector);
}


Arrow::Arrow(Surface* graph, QVector3D vector, QColor color) {
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
    float unitPlotPerGraph = (direction *
        QVector3D(m_graph->maxX - m_graph->minX, 0, m_graph->maxZ - m_graph->minZ)
        ).length();

    m_magnitude = magnitude;
    float magnitude_in_unit_arrow = magnitude * kUnitItemPerGraph / unitPlotPerGraph * kItemScale;
    setScaling(QVector3D(0.1, 0.1 * magnitude_in_unit_arrow, 0.1));
}


void Arrow::setPosition(const QVector3D & position){
    QCustom3DItem::setPosition(position);
    if (m_label != nullptr)
        m_label->setPosition(position + kLabelOffset
                             + renderedVectorInPlotUnit() / 2);
}


Square::Square(Surface* graph) : LabeledItem(graph){
    setMeshFile(QStringLiteral(":/mesh/plane.obj"));
    setScaling(QVector3D(0.1, 0.1, 0.1));
}

void Square::setArea(const float &area){
    setScaling(QVector3D(0.1 * sqrt(area), 0.1, 0.1 * sqrt(area)));
}
