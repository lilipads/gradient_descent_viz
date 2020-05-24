#ifndef ITEM_H
#define ITEM_H

#include <QtDataVisualization/QCustom3DItem>

using namespace QtDataVisualization;

class Item
{
public:
    Item();
};


class Ball : public QCustom3DItem
{
public:
    Ball(QColor color);
};

class Arrow : public QCustom3DItem{
public:
    Arrow(QVector3D direction);
};

#endif // ITEM_H
