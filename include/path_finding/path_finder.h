#pragma once

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QRectF>
#include <vector>

#include "qt_graphics_models/obstacle.h"
#include "path_step.h"
#include "path_planner.h"

namespace vrac::path_finding {

struct holonome;
struct differential;

class path_finder_signal_slots :public QObject{
    Q_OBJECT
public:
    path_finder_signal_slots(QObject *parent = nullptr) : QObject(parent) {}
signals :
    void blocked(); // Emitted when no path is found and blocked delay expired
    void emergency_stop(); // Emitted when the path collide with an ennemy
    void new_path_found(const std::vector<path_step> &, double theta_end);

public slots:
    virtual void set_obstacles(const std::vector<qt_graphics::models::obstacle> & obstacles) = 0;
    virtual void set_static_obstacles(const std::vector<qt_graphics::models::obstacle> & static_obstacles) = 0;
    virtual void set_current_pos(QPointF pos) = 0;
    virtual void find_new_path() = 0;

protected slots:
    virtual void update() = 0;
    virtual void move_finished() = 0;
};

using namespace std::chrono_literals;

template<typename drive_policy = differential>
class path_finder : public path_finder_signal_slots {

    friend holonome;
    friend differential;

public :

    path_finder(std::chrono::milliseconds delay_find_path = 200ms, std::chrono::milliseconds delay_blocked = 3000ms, std::chrono::milliseconds loop_period = 100ms, QRectF hitbox = {}, QObject *parent = nullptr) : path_finder_signal_slots(parent), hitbox(hitbox)
    {
        delay_timer.setInterval(delay_find_path.count());
        blocked_timer.setInterval(delay_blocked.count());
        connect(&delay_timer, &QTimer::timeout, this, &path_finder::find_new_path);
        connect(&blocked_timer, &QTimer::timeout, this, &path_finder::blocked);
        connect(&loop_timer, &QTimer::timeout, this, &path_finder::update);

        delay_timer.setSingleShot(true);
        blocked_timer.setSingleShot(true);
        loop_timer.start(loop_period);
    }

    virtual ~path_finder() {
        obstacles.clear();
        static_obstacles.clear();
        path.clear();
    }

    void set_ignore_static_obstacles(bool newIgnoreStaticObstacle) {ignore_static_obstacles = newIgnoreStaticObstacle;}
    void set_collision_hysteresis(int newCollisionHysteresis) {collision_hysteresis = newCollisionHysteresis;}
    
    std::vector<qt_graphics::models::obstacle> get_all_obstacles() {
        auto all_obstacles = obstacles;
        if (!ignore_static_obstacles) {
            all_obstacles.insert(all_obstacles.end(), static_obstacles.begin(), static_obstacles.end()) ;
        }
        return all_obstacles;
    }

    void set_new_goal(QPointF goal, double theta)
    {
        theta_end = theta;
        set_new_goal(path_step(current_pos, goal, hitbox.width()));
    }

    void set_new_goal(path_step path_step) {
        path = {path_step};
    }

    virtual void set_obstacles(const std::vector<qt_graphics::models::obstacle> & obs) override {
        obstacles.clear();
        obstacles = ranges::copy(obs);
    }
    virtual void set_static_obstacles(const std::vector<qt_graphics::models::obstacle> & s_obs) override { static_obstacles = ranges::copy(s_obs); }
    virtual void set_current_pos(QPointF pos) override {
        current_pos = pos;
        if (std::empty(path)) return;
        path.front().setStart(current_pos);
    }

    virtual void find_new_path() override
    {
        std::vector<path_step> new_path = path_planner::find_path(path.front().start, path.back().goal,  get_all_obstacles(), hitbox);

        if (std::empty(new_path)) {
            if (!blocked_timer.isActive()) {
                blocked_timer.start();
            }
            return;
        }

        if (blocked_timer.isActive()) {
            blocked_timer.stop();
        }

        path = new_path;
        emit new_path_found(path, theta_end);
    }

    virtual void update() override {

        if (std::empty(path)) return;

        if (path.front().ui_item->path().length() < 10) {
            move_finished();
        }

        drive_policy::update(*this);
    }

    void set_hitbox(const QRectF &newHitbox) { hitbox = newHitbox;}

    virtual void move_finished() override {path.erase(path.begin());}

    const std::vector<path_step> &get_path() const {return path;}
private :
    std::vector<qt_graphics::models::obstacle> obstacles;
    std::vector<qt_graphics::models::obstacle> static_obstacles;
    std::vector<path_step> path;

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

        if (!path_checker::check_path(path_finder.path, path_finder.get_all_obstacles(), path_finder.hitbox)) {
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

        if (path_checker::check_path(path_finder.path, path_finder.get_all_obstacles(), path_finder.hitbox)) {

            if (path_finder.collision_count > path_finder.collision_hysteresis) {
                path_finder.find_new_path();
            }

            path_finder.collision_count = 0;

            for (auto & step : path_finder.path) {
                step.ui_item->setPen(QPen(Qt::blue, path_finder.hitbox.width()));
            }

            if (path_finder.delay_timer.isActive()) {
                path_finder.delay_timer.stop();
            }
            return;
        }

        for (auto & step : path_finder.path) {
            step.ui_item->setPen(QPen(Qt::red, path_finder.hitbox.width()));
        }

        path_finder.collision_count++;
        if (path_finder.collision_count < path_finder.collision_hysteresis) {
            return;
        }
        emit path_finder.emergency_stop();
        if (!path_finder.delay_timer.isActive()) path_finder.delay_timer.start(); // Calc new Path at timeout
    }
};





}
