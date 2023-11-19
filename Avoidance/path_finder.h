#pragma once

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QRectF>
#include <QVector>

#include "Scenes/obstacle.h"
#include "pathstep.h"
#include "path_planner.h"

struct holonome;
struct differential;

class path_finder_signal_slots :public QObject{
    Q_OBJECT
public:
    path_finder_signal_slots(QObject *parent = nullptr) : QObject(parent) {}
signals :
    void blocked(); // Emitted when no path is found and blocked delay expired
    void emergency_stop(); // Emitted when the path collide with an ennemy
    void move_to(PathStep, double theta_end = 0xFFFF);
    void new_path_found(const QVector<PathStep> &);

public slots:
    virtual void set_new_goal(QPointF pos, double thetaEnd) = 0;
    virtual void set_obstacles(const QVector<Obstacle> & obstacles) = 0;
    virtual void set_static_obstacles(const QVector<Obstacle> & static_obstacles) = 0;
    virtual void set_current_pos(QPointF pos) = 0;
    virtual void find_new_path() = 0;

protected slots:
    virtual void update() = 0;
    virtual void move_finished() = 0;
};

template<typename drive_policy>
class path_finder : public path_finder_signal_slots {

    friend holonome;
    friend differential;

public :
    path_finder(QRectF hitbox, QObject *parent = nullptr) : path_finder_signal_slots(parent), hitbox(hitbox)
    {
        connect(&delay_timer, &QTimer::timeout, this, &path_finder::find_new_path);
        connect(&blocked_timer, &QTimer::timeout, this, &path_finder::blocked);
        connect(&loop_timer, &QTimer::timeout, this, &path_finder::update);

        delay_timer.setSingleShot(true);
        blocked_timer.setSingleShot(true);
        loop_timer.start(100);
    }

    void set_delay(int ms) {delay_timer.setInterval(ms); } // Delay before calculating a newPath
    void set_blocked_delay(int ms) {blocked_timer.setInterval(ms);} // Delay before sending the blocked signal when no path is found
    void set_holo_mode(bool mode) {holo_mode = mode;}
    void set_ignore_static_obstacles(bool newIgnoreStaticObstacle) {ignore_static_obstacles = newIgnoreStaticObstacle;}
    void set_collision_hysteresis(int newCollisionHysteresis) {collision_hysteresis = newCollisionHysteresis;}

    QVector<Obstacle> get_all_obstacles() {
        auto all_obstacles = obstacles;
        if (!ignore_static_obstacles) all_obstacles << static_obstacles;
        return all_obstacles;
    }

    virtual void set_new_goal(QPointF goal, double theta) override
    {
        theta_end = theta;
        QVector<PathStep> new_path = {PathStep(current_pos, goal, hitbox.width())};
        emit new_path_found(new_path);
    }
    virtual void set_obstacles(const QVector<Obstacle> & obs) override { obstacles = std::move(obs); }
    virtual void set_static_obstacles(const QVector<Obstacle> & s_obs) override { static_obstacles = std::move(s_obs); }
    virtual void set_current_pos(QPointF pos) override { current_pos = pos; }
    virtual void find_new_path() override
    {
        QVector<PathStep> newPath = path_planner::findPath(path, get_all_obstacles(), hitbox);

        if (newPath.isEmpty()) {
            if (!blocked_timer.isActive()) blocked_timer.start();
            return;
        }

        if (blocked_timer.isActive()) blocked_timer.stop();

        path = newPath;
        emit new_path_found(newPath);
    }
    virtual void update() override
    {
        drive_policy::update(*this);
    }
    virtual void move_finished() override
    {
        path.pop_front();
        emit move_to(path.front());
    }

private :
    QVector<Obstacle> obstacles;
    QVector<Obstacle> static_obstacles;
    QVector<PathStep> path;

    QPointF current_pos;
    QRectF hitbox;

    QTimer delay_timer;
    QTimer blocked_timer;
    QTimer loop_timer;

    bool ignore_static_obstacles = false;
    int collision_hysteresis = 2;
    int collision_radius = 1000;
    int collision_count = 0;
    int holo_mode = false;
    double theta_end = 0;
};

struct holonome {
    template<typename path_finder_t>
    static void update(path_finder_t & path_finder) {
        path_finder.find_new_path();

        if (!path_checker::checkPath(path_finder.path, path_finder.get_all_obstacles(), path_finder.hitbox)) {
            path_finder.collision_count++;

            if (path_finder.collision_count < path_finder.collision_hysteresis) {
                return;
            }

            emit path_finder.emergency_stop();
        }

        path_finder.collision_count = 0;
    }
};

struct differential {
    template<typename path_finder_t>
    static void update(path_finder_t & path_finder) {

        if (!path_checker::checkPath(path_finder.path, path_finder.get_all_obstacles(), path_finder.hitbox)) {
            path_finder.collision_count++;
            if (path_finder.collision_count < path_finder.collision_hysteresis) {
                return;
            }
            emit path_finder.emergency_stop();
            if (!path_finder.delay_timer.isActive()) path_finder.delay_timer.start(); // Calc new Path at timeout
        }
        path_finder.collision_count = 0;
        if (path_finder.delay_timer.isActive())
            path_finder.delay_timer.stop();
    }
};
