#ifndef ITEM_H
#define ITEM_H

#include <QtDataVisualization/QCustom3DItem>
#include <QtDataVisualization/QCustom3DLabel>

#include "surface.h"

using namespace QtDataVisualization;

// for 1 unit of magnitude, the rendered item spans this many units on the graph
// one might want to change this if gradient arrows rendered in 1:1 ratio is too
// big / small to see.
const float kItemScale = 0.1;
// how many unit arrows in one side of the graph
const float kUnitItemPerGraph = 110;
const QVector3D kLabelOffset(0, 20, 0); // label's position relative to the object's position


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

    QVector3D renderedVectorInPlotUnit(){
        /* the xyz coordinates of the rendered vector (starts from the origin)
         * scale down by a constant because when rendering, we don't draw it
         * to the full scale (otherwise it would dominate the screen)
         */
         return direction * m_magnitude * kItemScale;
    }
    float magnitude(){return m_magnitude;}
    void setPosition(const QVector3D& position);

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
