#ifndef PLAYGROUND_H
#define PLAYGROUND_H

#include <QGraphicsScene>
#include <QObject>

#include "Scenes/obstacle.h"
#include "Scenes/gameelement.h"
#include "Avoidance/pathstep.h"

class Playground : public QGraphicsScene
{
public:
    explicit Playground(QObject *parent = nullptr);
    virtual ~Playground();

    GameElement* popElement(QString groupName, GameElement *e);
    GameElement* popElement(QString groupName, QString name);
    GameElement* popElement(QString groupName, size_t index);

    const QMap<QString, QVector<GameElement*>> &elements() const { return mElements;}
    QVector<GameElement*> & getElementGroup(QString groupName);
    GameElement* getClosestElement(QString groupName, QPointF currentPos);

    const QVector<Obstacle>& obstacles() const {return mObstacles;}
    const QVector<Obstacle>& staticObstacles() const {return mStaticObstacles;}
    const QVector<PathStep>& path();

public slots:
    void clearElements();
    void addElement(QString groupName, GameElement* element);
    void removeElement(QString groupName, QString name);

    void setObstacles(const QVector<Obstacle> &newObstacles);
    void setStaticObstacles(const QVector<Obstacle> &newStaticObstacles);
    void setPath(const QVector<PathStep>& newPath);
    void popPathStep();
    void updatePath();

private:
    QMap<QString, QVector<GameElement*>> mElements;
    QVector<Obstacle> mObstacles;
    QVector<Obstacle> mStaticObstacles;
    QVector<PathStep> mPath;
};

#endif // PLAYGROUND_H
