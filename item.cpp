#include "item.h"

LabeledItem::LabeledItem()
{
    initializeLabel();
}

void LabeledItem::initializeLabel(){
    m_label = new QCustom3DLabel;
    m_label->setFacingCamera(true);
    m_label->setScaling(QVector3D(1., 1., 1.));
    m_label->setVisible(label_visibility && this->isVisible());
}

void LabeledItem::addToGraph(Q3DSurface *graph){
    graph->addCustomItem(this);
    graph->addCustomItem(m_label);
    m_graph = graph;
}


void LabeledItem::setColor(QColor color){
    QImage pointColor = QImage(2, 2, QImage::Format_ARGB32);
    pointColor.fill(color);
    setTextureImage(pointColor);
}


void LabeledItem::setLabel(const QString &text){
    // the graph somehow doesn't update the text on its own
    // so we need this roundabout way of removing the label
    // and reinitializing it
    if (m_graph != nullptr){
        m_graph->removeCustomItem(m_label);
        m_label = nullptr;
    }

    initializeLabel();
    m_label->setText(text);
    label_visibility = true;
    m_label->setVisible(label_visibility && this->isVisible());

    if (m_graph != nullptr){
        m_graph->addCustomItem(m_label);
    }
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


Ball::Ball(QColor color){
    setScaling(QVector3D(0.01f, 0.01f, 0.01f));
    setMeshFile(QStringLiteral(":/mesh/largesphere.obj"));
    setColor(color);
}


Arrow::Arrow(){
    setMeshFile(QStringLiteral(":/mesh/narrowarrow.obj"));
    setColor(Qt::black);
    setMagnitude(0);
}

Arrow::Arrow(QVector3D vector): Arrow() {
    setVector(vector);
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
    setScaling(QVector3D(0.1, kArrowYScale * magnitude, 0.1));
    m_magnitude = magnitude;
}

void Arrow::setPosition(const QVector3D &position){
    /* set position relative to the root of the arrow
     * (instead of the center). Should be called after scaling is done
     */

    QCustom3DItem::setPosition(position + vector() / 2.);
    m_label->setPosition(position + kLabelOffset + vector() / 2);
}
