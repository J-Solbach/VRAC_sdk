#pragma once

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QRectF>
#include <vector>
#include <QTransform>

#include "qt_graphics_models/obstacle.h"
#include "qt_graphics_models/game_element.h"
#include "path_step.h"
#include "path_planner.h"

#include <spdlog/spdlog.h>
#include <fmt/ranges.h>

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
    void escape_procedure(double dist, bool forward);

public slots:
    virtual void set_obstacles(const std::vector<qt_graphics::models::obstacle> & obstacles) = 0;
    virtual void set_static_obstacles(const std::vector<qt_graphics::models::obstacle> & obstacles) = 0;
    virtual void add_static_obstacle(const qt_graphics::models::obstacle & obstacles) = 0;

    virtual void set_game_elements(const std::vector<qt_graphics::models::game_element> & game_elements) = 0;

    virtual void set_current_pos(QPointF pos) = 0;
    virtual void set_emergency_stop_status(bool) = 0;
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

    path_finder(std::chrono::milliseconds delay_blocked = 2000ms, std::chrono::milliseconds loop_period = 100ms, QRectF hitbox = {}, QObject *parent = nullptr) : path_finder_signal_slots(parent), hitbox(hitbox), loop_period(loop_period)
    {
        blocked_timer.setInterval(delay_blocked.count());
        resend_path_timer.setInterval(delay_blocked.count() * 2);
        connect(&blocked_timer, &QTimer::timeout, this, &path_finder::blocked);
        connect(&pause_timer, &QTimer::timeout, this, &path_finder::resume);
        connect(&loop_timer, &QTimer::timeout, this, &path_finder::update);
        connect(&resend_path_timer, &QTimer::timeout, this, &path_finder::find_new_path);
        connect(&resend_path_timer, &QTimer::timeout, this, [&](){
            spdlog::warn("Resending path since you are dumb dumb!");
        });

        resend_path_timer.setSingleShot(true);
        blocked_timer.setSingleShot(true);
        pause_timer.setSingleShot(true);
        loop_timer.start(loop_period);
    }

    virtual ~path_finder() {
        obstacles.clear();
        static_obstacles.clear();
        game_elements.clear();
        path.clear();
    }

    void reset() {
        path.clear();
        obstacles.clear();
        static_obstacles.clear();
        game_elements.clear();
    }

    void pause(std::chrono::milliseconds pause_duration) {
        loop_timer.stop();
        if(pause_timer.isActive()) pause_timer.stop();

        pause_timer.start(pause_duration.count());
    }

    void resume() {
        spdlog::info("Resuming path findind after pause");
        loop_timer.start(loop_period);
        find_new_path();
    }

    void set_ignore_static_obstacles(bool newIgnoreStaticObstacle) {ignore_static_obstacles = newIgnoreStaticObstacle;}
    void set_ignore_game_elements(bool newIgnoreGameElements) {ignore_game_elements = newIgnoreGameElements;}
    void set_collision_hysteresis(int newCollisionHysteresis) {collision_hysteresis = newCollisionHysteresis;}

    std::vector<qt_graphics::models::obstacle> get_all_obstacles() {
        auto all_obstacles =  obstacles;
        if (!ignore_static_obstacles) {
            all_obstacles.insert(all_obstacles.end(), static_obstacles.begin(), static_obstacles.end()) ;
        }

        if (!ignore_game_elements) {
            all_obstacles.insert(all_obstacles.end(), game_elements.begin(), game_elements.end()) ;
        }

        return all_obstacles;
    }

    std::vector<qt_graphics::models::obstacle> get_static_obstacles() {
        return static_obstacles;
    }

    std::vector<qt_graphics::models::obstacle> get_game_elements() {
        return game_elements;
    }

    void set_new_goal(QPointF goal, double theta)
    {
        set_new_goal(path_step(current_pos, goal, hitbox.width()), theta);
    }

    void set_new_goal(path_step path_step, double theta) {
        theta_end = theta;
        spdlog::info("[path finder] new goal received : {} {} {}", path_step.goal.x(), path_step.goal.y(), theta_end);
        path = {path_step};
    }

    void set_path(const auto & new_path, double theta) {
        spdlog::debug("[path finder] new path set : {}", new_path);

        theta_end = theta;
        path = new_path;
    }

    virtual void set_obstacles(const std::vector<qt_graphics::models::obstacle> & obs) override {
        obstacles.clear();
        obstacles = ranges::copy(obs);
    }

    virtual void set_static_obstacles(const std::vector<qt_graphics::models::obstacle> & s_obs) override {
        static_obstacles.clear();
        static_obstacles = ranges::copy(s_obs);
    }

    virtual void add_static_obstacle(const qt_graphics::models::obstacle & s_ob) override {
        static_obstacles.push_back(s_ob);
    }

    virtual void set_game_elements(const std::vector<qt_graphics::models::game_element> & new_game_elements) override {
        game_elements.clear();
        game_elements = new_game_elements
        | ranges::views::transform([](const auto & game_element){
            return game_element.hitbox;
        })
        | ranges::to<std::vector>;
    }


    void add_game_elements(const qt_graphics::models::game_element & new_game_element) {
        game_elements.emplace_back(new_game_element.hitbox);
    }

    virtual void set_current_pos(QPointF pos) override {
        current_pos = pos;
        if (std::empty(path)) return;
        path.front().setStart(current_pos);
    }

    const QPointF & get_current_pos() const {return current_pos;}

    virtual void set_emergency_stop_status(bool sts) override {

        if (emergency_stop_status != sts) {
            spdlog::warn("Emergency stop status changed to {}", (sts) ? "active" : "unactive");
        }

        if (sts && resend_path_timer.isActive()) {
            resend_path_timer.stop();
        }

        emergency_stop_status = sts;
    }
    bool get_emergency_stop_status() {return emergency_stop_status;}

    virtual void find_new_path() override
    {
        std::vector<path_step> new_path = path_planner::find_path(path.front().start, path.back().goal,  get_all_obstacles(), hitbox);

        if (std::empty(new_path)) {

            if (!blocked_timer.isActive()) {
                blocked_timer.start();
            }
            return;
        }

        spdlog::debug("[path finder] new path found : {}", new_path);

        if (blocked_timer.isActive()) {
            blocked_timer.stop();
        }

        resend_path_timer.start();
        path = std::move(new_path);
        emit new_path_found(path, theta_end);
    }

    virtual void update() override {

        if (std::empty(path)) return;

        if (path.front().ui_item->path().length() < 5) {
            move_finished();
        }

        drive_policy::update(*this);
    }

    void try_escape_from(QPointF obstacle_position, double current_theta, double dist, bool with_lidar = false) {
        QTransform transform;
        // Apply rotation around the robot's position
        transform.translate(current_pos.x(), current_pos.y());
        transform.rotate(-current_theta + 90);

        // Translate the point from robot's reference frame to playground's reference frame
        QPointF forward_point = QPointF(dist, 0);
        QPointF backward_point = QPointF(-dist, 0);

        auto escape_forward = transform.map(forward_point);
        auto escape_backward = transform.map(backward_point);

        bool forward = QLineF(escape_forward, obstacle_position).length() > QLineF(escape_backward, obstacle_position).length();
        auto escape_pos = (forward)? escape_forward : escape_backward;

        auto forced_all_obstacles = (with_lidar)
        ? ranges::views::concat(obstacles, static_obstacles, game_elements) | ranges::to<std::vector>
        : ranges::views::concat(static_obstacles, game_elements) | ranges::to<std::vector>;

        while (!path_checker::check_path(current_pos, escape_pos, forced_all_obstacles, hitbox) && dist > 10) {
            spdlog::error("No way to escape to {} collising with an obstacle with lidar : {}", escape_pos, with_lidar);
            dist = dist/2;
        }

        if (!path_checker::check_path(current_pos, escape_pos, forced_all_obstacles, hitbox)) {
            spdlog::error("No way to escape to {} collising with an obstacle with lidar : {}", escape_pos, with_lidar);
            return;
        }

        spdlog::info("Line escape valid {} mm forward: {}", dist, forward);

        using namespace std::chrono_literals;
        pause(1000ms);

        emit escape_procedure(dist, forward);
    }

    void set_hitbox(const QRectF &newHitbox) { hitbox = newHitbox;}
    const QRectF & get_hitbox() const {return hitbox;}

    virtual void move_finished() override {
        path.erase(path.begin());
        if (resend_path_timer.isActive()) resend_path_timer.stop();

        if (std::ranges::size(path) > 0) {
            resend_path_timer.start();
        }
    }

    const std::vector<path_step> & get_path() const {return path;}
    double get_theta_end() const {return theta_end;}

private :
    std::vector<qt_graphics::models::obstacle> obstacles;
    std::vector<qt_graphics::models::obstacle> static_obstacles;
    std::vector<qt_graphics::models::obstacle> game_elements;
    std::vector<path_step> path;

    QPointF current_pos;
    QRectF hitbox;

    QTimer blocked_timer;
    QTimer pause_timer;
    QTimer loop_timer;
    QTimer resend_path_timer;

    std::chrono::milliseconds loop_period;

    bool ignore_static_obstacles = false;
    bool ignore_game_elements = false;
    bool emergency_stop_status = false;
    int collision_hysteresis = 2;
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
        try {
            if (path_finder.get_emergency_stop_status()){
                path_finder.find_new_path();
                return;
            }

            if (!path_checker::check_path(path_finder.path, path_finder.get_all_obstacles(), path_finder.hitbox)) {
                path_finder.collision_count++;

                if (path_finder.collision_count < path_finder.collision_hysteresis) {
                    return;
                }

                for (auto & step : path_finder.path) {
                    step.ui_item->setPen(QPen(Qt::red, path_finder.hitbox.width()));
                }

                spdlog::warn("[path finder] path collide with an obstacle ! Searching a new patch");
                emit path_finder.emergency_stop();
            }
            path_finder.collision_count = 0;
        }
        catch(const std::out_of_range & e) {
            spdlog::error("[path finder] out_of_range error during update {}", e.what());
        }
        catch(const std::runtime_error & e) {
            spdlog::error("[path finder] runtime error during update {}", e.what());
        }
        catch(const std::exception & e) {
            spdlog::error("[path finder] exception catched during update {}", e.what());
        }
        catch(...) {
            spdlog::error("[path finder] unknown error during update");
        }
    }
};
}
