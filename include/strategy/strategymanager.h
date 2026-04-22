#pragma once

#include <QObject>
#include <QTimer>

class strategy_manager_signals : public QObject {
    Q_OBJECT
public:
    explicit strategy_manager_signals(QObject *parent = nullptr) : QObject(parent) {}
signals:
    void do_funny_action();
    void go_back_home();
    void end();

public slots:
    virtual void jack_pulled() = 0;
};

template<typename strategy_type>
class strategyManager : public strategy_manager_signals
{
public:

    strategyManager(strategy_type * strategy, QObject *parent = nullptr) :
        strategy_manager_signals(parent),
        strategy(strategy)
    {
        game_timer.setInterval(std::chrono::seconds(100));
        go_back_home_timer.setInterval(std::chrono::seconds(92));
        funny_action_timer.setInterval(std::chrono::seconds(98));

        game_timer.setSingleShot(true);
        go_back_home_timer.setSingleShot(true);
        funny_action_timer.setSingleShot(true);

        connect(&game_timer, &QTimer::timeout, this, &strategyManager::end);
        connect(&game_timer, &QTimer::timeout, this, [&](){game_ended = true;});
        connect(&go_back_home_timer, &QTimer::timeout, this, &strategyManager::go_back_home);
        connect(&funny_action_timer, &QTimer::timeout, this, &strategyManager::do_funny_action);
    }

    virtual ~strategyManager() {
        delete strategy;
    }

    void set_do_funny_action_time(std::chrono::seconds seconds) { funny_action_timer.setInterval(std::chrono::seconds(seconds)); }
    void set_go_back_home_time(std::chrono::seconds seconds) { go_back_home_timer.setInterval(std::chrono::seconds(seconds)); }
    bool has_game_ended() const { return game_ended; }

    virtual void jack_pulled() override {
        if(game_timer.isActive() || game_ended) return;

        game_timer.start();
        go_back_home_timer.start();
        funny_action_timer.start();
    }

    void start() {
        strategy->start();
    }

    strategy_type *getStrategy() const
    {
        return strategy;
    }

    std::chrono::seconds get_remaining_game_time() {
        return std::chrono::duration_cast<std::chrono::seconds>(game_timer.remainingTimeAsDuration());
    }

private:
    QTimer game_timer;
    QTimer go_back_home_timer;
    QTimer funny_action_timer;

    strategy_type *strategy;

    bool game_ended = false;
};


