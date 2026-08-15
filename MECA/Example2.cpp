#include <meca.h>
#include <logger.h>


struct Health {
    int value = 100;
};

struct Position {
    int x = 0;
    int y = 0;
};


int main() {    
    meca::componentRegistry<Health> healths;
    meca::componentRegistry<Position> positions;

    //Creating entities
    auto start_creating = TimeShot();

    for(int i = 0; i < 10000; i++) {
        meca::create_entity();
    }

    auto end_creating = TimeShot();

    Logger(LOGGER_INFO, "MAIN", elapsed_time(start_creating, end_creating, LOGGER_MICRO_SECONDS));


    //Creating components
    auto start_comps = TimeShot();

    for(int i = 0; i < 10000; i++) {
        meca::create_component(i, {}, healths);
        meca::create_component(i, {}, positions);
    }

    auto end_comps = TimeShot();

    Logger(LOGGER_INFO, "MAIN", elapsed_time(start_comps, end_comps, LOGGER_MICRO_SECONDS));


    //System tests
    auto start_system = TimeShot();

    for(Health &hp : meca::component_iterator(healths)) {
        hp.value += 50;
    }

    meca::filter_for(
        meca::AND_E,
        [](Health &hp, Position &pos) {
            pos.x += hp.value;
            pos.y = pos.x*2;
        },
        healths, positions
    );

    auto end_system = TimeShot();

    Logger(LOGGER_INFO, "MAIN", elapsed_time(start_system, end_system, LOGGER_MICRO_SECONDS));
}