#pragma once

#include <QThread>
#include <QObject>
#include <QTimer>
#include <range/v3/all.hpp>

#include "sensor.h"
#include "qt_graphics_models/obstacle.h"

class DetectionManager : public QThread
{
    Q_OBJECT
public :
    DetectionManager(qreal ennemy_radius, qreal robot_radius, QObject *parent = nullptr)
        : QThread(parent),
        mEnnemyRadius(ennemy_radius),
        mRobotRadius(robot_radius)
    {
        connect(&mLoopTicker, &QTimer::timeout, this, &DetectionManager::update);
    }

    ~DetectionManager()
    {
        for (auto sensor : mBodySensors) {
            delete sensor;
        }
        mBodySensors.clear();

        for (auto sensor : mBeaconSensors) {
            delete sensor;
        }
        mBeaconSensors.clear();
    }

    virtual void run() override
    {
        mLoopTicker.start(100);
        exec();
    }

    static void addToGroup(QPointF point, std::vector<QPolygonF>& groups)
    {
        auto detectedGroup = ranges::find_if(groups, [&point](const auto & group){
            QLineF dist = QLineF(group.boundingRect().center(), point);
            return (dist.length() < 50);
        });

        if (detectedGroup == std::end(groups)) {
            QPolygonF poly;
            poly.append(point);
            groups.push_back(std::move(poly));
        }
        else {
            detectedGroup->push_back(point);
        }
    }


    void addBodySensor(ISensor *sensor) { mBodySensors.push_back(sensor); }
    void addBeaconSensor(ISensor *sensor) { mBeaconSensors.push_back(sensor); }
    void setEnnemyRadius(qreal newEnnemyRadius) { mEnnemyRadius = newEnnemyRadius; }
    void setRobotRadius(qreal newRobotRadius) { mRobotRadius = newRobotRadius; }

signals:
    void newObstacles(std::vector<obstacle>);

private slots:
    void update()
    {
        const auto & make_grouped_points = [](const std::vector<ISensor *> & sensors) -> std::vector<QPolygonF>{
            std::vector<QPolygonF>groupedPoints;
            auto sensor_data_as_pointF = sensors
                                         | ranges::views::transform([](auto &sensor){
                                               return sensor->read();
                                           })
                                         | ranges::views::join
                                         | ranges::views::transform([](auto &data){
                                               return data.toPointF();
                                           })
                                         | ranges::to<std::vector>;
            ranges::for_each(sensor_data_as_pointF, [&](const auto &point){
                addToGroup(point, groupedPoints);
            });

            return groupedPoints;
        };

        std::vector<QPolygonF>groupedBeaconPoints = make_grouped_points(mBeaconSensors);
        std::vector<QPolygonF>groupedBodyPoints = make_grouped_points(mBodySensors);
        
        std::vector<obstacle> ennemies;
        qreal minDist = 100; // to be determined
        // now we have to pair bodypoints and beacon points
        for (auto & beaconGroup : groupedBeaconPoints) {
            QPolygonF ennemyBody;
            for (auto & bodyGroup : groupedBodyPoints) {
                qreal dist = QLineF(beaconGroup.boundingRect().center(), bodyGroup.boundingRect().center()).length();

                if (dist < minDist) {
                    ennemyBody.append(bodyGroup);
                }
            }

            ennemies.emplace_back(beaconGroup.boundingRect().center(), mEnnemyRadius, mRobotRadius, ennemyBody);
        }
        emit newObstacles(ennemies);
    }

private:

    QTimer mLoopTicker;
    std::vector<ISensor*> mBeaconSensors;
    std::vector<ISensor*> mBodySensors;

    qreal mEnnemyRadius;
    qreal mRobotRadius;
};
