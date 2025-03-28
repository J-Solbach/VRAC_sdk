#pragma once

#include <QObject>
#include <set>
#include <functional>
#include <range/v3/all.hpp>

#include "event.h"
#include "transition.h"

namespace vrac::strategy::state_machines {

class state_signals_slots :  public QObject {
    Q_OBJECT
public:
    explicit state_signals_slots(){
    }

signals :
    void send_event(state_machines::event & e);
public slots:
    virtual std::vector<std::string> on_event(state_machines::event & e) = 0;
};

// TODO RAII
template<typename context_t, typename params_t>
class state : public state_signals_slots
{
public:
    using context_type = context_t;
    using params_type = params_t;
    using signal_slot_handler_type = state_signals_slots;

    state(std::string name) :
        state_signals_slots(),
        name {"[" + name + "]"},
        checksum(std::hash<std::string>{}(name))
    {
        checksum += (rand() %0xFFFF);
    }

    state(const std::string & name,  const params_t & params) :
        state_signals_slots(), name {"[" + name + "]"},
        params(params), checksum(std::hash<std::string>{}(name))
    {
        checksum += (rand() %0xFFFF);
    }

    const std::vector<transition> &get_transitions() const { return transitions; }
    void set_transitions(std::vector<transition> && new_transitions) { transitions = std::move(new_transitions); }

    bool test_checksum(uint16_t incoming_checksum) {
        if (incoming_checksum == 0xFFFF) return true;
        if (incoming_checksum != checksum ) {
            spdlog::info("incoming checksum is not equal to requested checksum {} != {}", incoming_checksum, checksum);
            return false;
        }

        if (check_counter > 0) {
            spdlog::info("check_counter reduced ! remaining checks : {}", check_counter);
            check_counter--;
        }

        return (check_counter == 0);
    }

    virtual std::vector<std::string> on_event(state_machines::event & e) override {
        return transitions
               | ranges::views::filter([&](const auto & transition){
                     return transition.event == e && test_checksum(e.checksum);
                    })
                | ranges::views::transform([](const auto & transition) {
                     return transition.target_state;
                    })
                | ranges::to<std::vector>;
    }

    virtual void on_entry(context_t &, state_machines::event) = 0;
    virtual void on_exit(context_t &, state_machines::event) {}

    params_t get_params() const {return params;}
    std::string get_name() const {return name;}
    uint16_t get_checksum() {
        return checksum;
    }

    void set_check_counter(std::size_t newCheck_counter) {check_counter = newCheck_counter;}

protected:
    std::string name;
    std::vector<transition> transitions;
    params_t params;

    uint16_t checksum = 0xFFFF;
    std::size_t check_counter = 1;
};

}

#include <fmt/core.h>
#include <fmt/format.h>

template<typename context_t, typename params_t>
struct fmt::formatter<vrac::strategy::state_machines::state<context_t, params_t>> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    static auto format(const vrac::strategy::state_machines::state<context_t, params_t> & val, format_context& ctx) {
        return fmt::format_to(ctx.out(), "{}", val.get_name());
    }
};

template<typename context_t, typename params_t>
struct fmt::formatter<vrac::strategy::state_machines::state<context_t, params_t>*> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    static auto format(vrac::strategy::state_machines::state<context_t, params_t> * val, format_context& ctx) {
        return fmt::format_to(ctx.out(), "{}", val->get_name());
    }
};




