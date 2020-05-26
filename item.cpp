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
    m_label->setVisible(label_visibility && isVisible());
}


void LabeledItem::setLabelVisibility(bool visible){
    label_visibility = visible;
    m_label->setVisible(visible && this->isVisible());
}

void LabeledItem::setPosition(const QVector3D & position){
    QCustom3DItem::setPosition(position);
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
    setMeshFile(QStringLiteral(":/mesh/narrowarrow.obj"));
    setMagnitude(0);
    setColor(color);
    setVector(vector);
    addToGraph(graph);
}


void Arrow::setVector(QVector3D vector){
    /* draw an arrow representing the vector (direction and magnitude) */

    // negative 1 because oddly, the default xyz axis
    // in Q3DSurface has Z axis flipped
    direction = vector.normalized();
    vector.setZ(-vector.z());
    QQuaternion rotation = QQuaternion::rotationTo(
                QVector3D(0, 1, 0), vector);
    setRotation(rotation);
    setMagnitude(vector.length());
}

void Arrow::setMagnitude(const float &magnitude){
    // if magnitude is negative, arrow extends in the other direction
    setScaling(QVector3D(0.1, 0.1 * magnitude, 0.1));
    m_magnitude = magnitude;
}


Square::Square(Surface* graph) : LabeledItem(graph){
    setMeshFile(QStringLiteral(":/mesh/plane.obj"));
    setScaling(QVector3D(0.1, 0.1, 0.1));
}

void Square::setArea(const float &area){
    setScaling(QVector3D(0.1 * sqrt(area), 0.1, 0.1 * sqrt(area)));
}
