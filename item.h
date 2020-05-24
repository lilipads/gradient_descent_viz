#ifndef ITEM_H
#define ITEM_H

#include <QtDataVisualization/QCustom3DItem>

using namespace QtDataVisualization;

const float kArrowOffset = 0.2;

class LabeledItem : public QCustom3DItem
{
public:
    LabeledItem();
    void setColor(QColor color);
};


class Ball : public LabeledItem
{
public:
    Ball(QColor color);
};

class Arrow : public LabeledItem{
public:
    Arrow();
    Arrow(QVector3D vector);
    void setVector(QVector3D vector);
    void setMagnitude(const float &magnitude);
    void setPosition(const QVector3D &position);

private:
    QVector3D direction = QVector3D(0, 1, 0);
    float magnitude = 1.0;

};

#endif // ITEM_H
