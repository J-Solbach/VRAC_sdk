QT += gui
QT += widgets
QT += concurrent
QT += network

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Avoidance/avoidancemanager.cpp \
    Avoidance/pathchecker.cpp \
    Avoidance/pathplanner.cpp \
    Avoidance/pathstep.cpp \
    Communications/messagePublisher.cpp \
    Communications/simulationsocket.cpp \
    Detection/detectionmanager.cpp \
    Detection/detectionutilities.cpp \
    Detection/sensor.cpp \
    JSON_overlay/jsonstatemachine.cpp \
    Scenes/obstacle.cpp \
    Scenes/playground.cpp \
    Scenes/robot.cpp \
    Strategy/Actions/actionfactory.cpp \
    Strategy/Actions/basicactions.cpp \
    Strategy/STM/state.cpp \
    Strategy/STM/stm.cpp \
    Strategy/STM/stmmanager.cpp \
    Strategy/STM/transition.cpp \
    Strategy/strategymanager.cpp

HEADERS += \
    Avoidance/avoidancemanager.h \
    Avoidance/motioncontroller.h \
    Avoidance/pathchecker.h \
    Avoidance/pathplanner.h \
    Avoidance/pathstep.h \
    Communications/messagePublisher.h \
    Communications/simulationsocket.h \
    Communications/subscriber.h \
    Detection/detectionmanager.h \
    Detection/detectionutilities.h \
    Detection/sensor.h \
    JSON_overlay/jsonstatemachine.h \
    Scenes/playground.h \
    Scenes/robot.h \
    Strategy/Actions/actionfactory.h \
    Strategy/Actions/basicactions.h \
    Strategy/STM/event.h \
    Strategy/STM/state.h \
    Strategy/STM/stm.h \
    Strategy/STM/stmmanager.h \
    Strategy/STM/transition.h \
    Utils/utils.h \
    Scenes/obstacle.h \
    gamestate.h \
    Scenes/gameelement.h \
    Strategy/GOAP/goal.h \
    Strategy/GOAP/planner.h \
    Strategy/GOAP/goapmanager.h \
    Strategy/GOAP/task.h \
    Strategy/strategymanager.h \
    Utils/Singleton.h \

QMAKE_CXXFLAGS += -Werror

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target
