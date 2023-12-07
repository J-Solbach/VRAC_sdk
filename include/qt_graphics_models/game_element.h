#pragma once

#include <QGraphicsItem>

class game_element : public QGraphicsItem
{
public:
    explicit game_element(QGraphicsItem* parent = nullptr) : QGraphicsItem(parent) {}
    explicit game_element(std::string name) : mName{name}{}
    virtual ~game_element() {}

    void set_entry_point(const QPointF & point) {entry_point = point;}
    QPointF get_entry_point() const { return entry_point;}

    virtual QRectF boundingRect() const override { return QRectF();}
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget * = nullptr)  override {}

    const std::string &name() const { return mName; }

private:
    QPointF entry_point;
    std::string mName;
};
