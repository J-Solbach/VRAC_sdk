# VRAC Eurobot SDK

This Sdk is an header only library, based on QT and used by the VRAC team to program their robots for the Eurobot competition.

## Features

- strategy: Allow the user to setup a strategy manager with either a state machine or a Goal oriented action Planning (GOAP) AI policy.

- path_finding: Can either be used with a differential or an holonomous policy. The user provides (via signals) the robot, goal and obstacles on the playground and the path finder will answer the shortest path to the goal.

- Detection: Create and add either beacon or body sensors to the detection_manager. It will gather the data periodically and generate obstacles on the playground. You can connect it to the path finder and the playground.

- qt_graphics_models: Collection of QGraphics models (playground, robot, obstacle and game_element) you can use to setup the UI.

- JSON_overlay: Creation of state machines via json files generated by our [strategy editor](https://github.com/J-Solbach/VRAC_strategy_editor) to use in the strategy Manager. You have to specialize the action_factory with the context you want to use in the STM.

## Libs

To ensure support for C++17, we use 4 header only libraries (fmt and range v3 are now part of the stl but only from c++20).

- range-v3: https://github.com/ericniebler/range-v3
- nlohmnan-json: https://github.com/nlohmann/json
- fmt: https://github.com/fmtlib/fmt
- spdlog: https://github.com/gabime/spdlog

## Getting started

### Prerequisites
C++ 17 is required as well as QT > 5.

### Install
This SDK is header only so you only need to copy the include directory in your Qt project.

```bash
$ git clone https://github.com/J-Solbach/VRAC_sdk.git
$ cd <your project directory>
$ cp -r <path to sdk>/include .
```

then add the following lines in your .pro file to include the headers in your project.

```
INCLUDEPATH += \
    <path_to_sdk>/include \
    <path_to_sdk>/libs \
    <path_to_sdk>/libs/range-v3/include \
    <path_to_sdk>/libs/nlohmann-json/include \
    <path_to_sdk>/libs/fmt/include \

```

or add it to the target include directories if you are using CMake.

## Usage

### Settting up a playground

To easily create a view of the playground with Qt you need to setup a QGraphicsView in your .ui file.

Then add the following code :

```c++
// In a file with your ui constants
constexpr std::string_view playground_image_resource = ":/UI/Images/vinyle_table.svg";
constexpr unsigned int playground_width = 3000; // in mm
constexpr unsigned int playground_height = 2000; // in mm
constexpr double scaling = 0.2; // scaling between QGraphicsView and reality

// In your mainwindow.cpp file
#include "qt_graphics_models/playground.h"

QRectF sceneRect(0, 0, playground_width, playground_height);
QImage image = QIcon(playground_image_resource.data()).pixmap(sceneRect.width(), sceneRect.height()).toImage();

Playground playground; // should be part of your member variables (if deleted, the ui won't show anything)
playground.setSceneRect(sceneRect);
playground.setBackgroundBrush(image);
ui->playground->setAttribute(Qt::WA_AcceptTouchEvents); // only usefull if you want to move your robot/obstacles via the mouse
ui->playground->scale(scaling, scaling);
ui->playground->setScene(&playground);
```

### setting up your robot

To add your robot on the playground just do this :

```c++
// in a file for your ui constants
constexpr std::string_view robot_image_resource = ":/UI/Images/robot.png";
constexpr double robot_scaling = 5; // how many times you need to multiply the img size to have 1pixel =  1mm in real life

// in your mainwindow.cpp
#include "qt_graphics_models/robot.h"
QPixmap robotPixmap = QPixmap(robot_image_resource.data());
Robot robot; // should be part of your member variables (if deleted, the ui won't show anything)
robot.setPixmap(robotPixmap.scaled(robotPixmap.width() * robot_scaling, robotPixmap.height() * robot_scaling));
robot.setPos(QPointF(500,500)); // initial pose (0,0) if not setup
playground.addItem(&robot);
```

### strategy

To setup a strategy manager, you have three choices. You can use a state machine (either manualy setup using the STM/stm.h, STM/state.h etc. or via the JSON_overlay if you are also using our [strategy editor](https://github.com/J-Solbach/VRAC_strategy_editor)) or you can use the GOAP AI.

#### How to setup a state machine manualy
TODO

#### How to setup a json state machine

First, you need to create your context. The state machine needs to have access to a context which allows it to interact with the rest of the program you wrote.

You can add whatever you need in this context.

```c++
struct VRAC_context {
    bool colorside;
    path_finder & path_finder;
    CanBusSocket can;
    //...
};
```

When your context is created you can start to write Actions which inherit from a ```state<your_context_type, nlohmann::json>``` and override the onEntry and if needed the onExit.

Exemple with the `Line` action :
```c++
struct Line : public state<VRAC_context, nlohmann::json> {
public:
    Line(const std::string & name,  const nlohmann::json & params) : state<VRAC_context, nlohmann::json>(name, params){
    }

    virtual void onEntry(VRAC_context &, Event) override {
        // read params to get the distance
        // send goal to path_finder to check if path is ok
        // send command to can
    }

    virtual void onExit(VRAC_context &, Event) override {
    }
};
```

When you have both the context and some actions. you can now setup your `action_factory`. It's a struct provided by the JSON_overlay which takes a `context_type` as a template parameter. You need to specialize the template with the following syntax:

```c++
template<>
action_factory<VRAC_context>::meta_factory_type action_factory<VRAC_context>::meta_factory = {
    {"Line",    [](const std::string & tag, const nlohmann::json &params) {return new Line(tag, params);} },
    {"YourActionName", [](const std::string & tag, const nlohmann::json &params) {return new YourActionName(tag, params);} },
    ...
};
```

This will make the function `make_stm_from_json` use the factory you setup to create the stm from the json file.

You can now create the stm from a json file and passe it to the strategy manager with the following code :

```c++
VRAC_context ctx {
    .colorside = BLUE_SIDE,
    .path_finder = path_finder,
    .can = can_socket
};

auto stm = make_stm_from_json<action_factory<context_vrac>>(ctx, "strat_name", "strat_directory");
auto manager = new strategyManager(stm);
```

### path finding

The path finding part needs a little setup to function. First you need to setup a robot and a playground from the qt_graphics_model. Then you have to select a policy (adapted to your motor drive system), setup the robot boundingRect and its position. Then you can connect the slots of the path finder.

```c++
path_finder<holonome> pf; // or path_finder<differential> path_finder;
pf.set_hitbox(robot.boundingRect());
pf.set_current_pos(robot.pos());

connect(&foo, &foo::newGoal, &pf, &path_finder<holonome>::set_new_goal); //
connect(&bar, &bar:::newObstacles, &pf, &path_finder<holonome>::set_obstacles); // Detection Manager
connect(this, &MainWindow::newObstacles, &playground, &Playground::onNewObstacles);
connect(&pf, &path_finder<selected_policy>::new_path_found, &playground, &Playground::onnew_path);
connect(&robot, &Robot::posChanged, &pf, &path_finder<holonome>::set_current_pos);
```


### logging

The logging part is quick and easy to setup. You only need to write the following code and thats it ! 

```c++
// IN your main file
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

int main(int argc, char *argv[])
{
    auto logger = spdlog::basic_logger_mt("vrac_logger", QDir::homePath().toStdString() + "/VRAC/logs/testlog.txt", true);
    spdlog::set_default_logger(logger);
    spdlog::flush_every(std::chrono::milliseconds(250));
}


// When you need to log something :
spdlog::info("Some info");
spdlog::warn("the action {} failed", action_name);
spdlog::error("");
// ETC.


```




