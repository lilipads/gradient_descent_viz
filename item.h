#ifndef ITEM_H
#define ITEM_H

#include <QtDataVisualization/QCustom3DItem>
#include <QtDataVisualization/QCustom3DLabel>

#include "surface.h"


using namespace QtDataVisualization;

const float kArrowYScale = 0.1; // 0.02; // how much we want to scale down the arrow
const float kArrowOffset = 8; // original arrow dimension in y axis
const QVector3D kLabelOffset(0, 20, 0);


class Item : public QCustom3DItem{
public:
    Item(Surface* graph) : m_graph(graph){
        m_graph->addCustomItem(this);
    }

protected:
    Item(){}
    QCustom3DLabel* m_label = nullptr;
    Surface* m_graph = nullptr;

    void setColor(QColor color);
    void addToGraph(Surface* graph);
};


class LabeledItem : public Item
{
public:
    LabeledItem(Surface* graph) : Item(graph){};

    void setLabel(const QString &text);
    void setLabelVisibility(bool visible);
    void setVisible(bool visible);
    void setPosition(const QVector3D& position);

protected:
    LabeledItem(){}
    bool label_visibility = false;
};


class Ball : public Item
{
public:
    Ball(Surface* graph, QColor color);
};


class Arrow : public LabeledItem{
public:
    Arrow(Surface* graph);
    Arrow(Surface* graph, QVector3D vector);
    Arrow(Surface* graph, QVector3D vector, QColor color);
    void setVector(QVector3D vector);
    void setMagnitude(const float &magnitude);

    QVector3D vector(){
        /* the xyz coordinates of the rendered vector (starts from the origin)
         * scale down by a constant because when rendering, we don't draw it
         * to the full scale (otherwise it would dominate the screen)
         */

         return direction * m_magnitude * kArrowYScale * kArrowOffset;}
    float magnitude(){return m_magnitude;}

private:
    QVector3D direction = QVector3D(0, 1, 0);
    float m_magnitude = 1.0;

};

class Square : public LabeledItem{
public:
    Square(Surface* graph);
    void setArea(const float &area);

};

#endif // ITEM_H
