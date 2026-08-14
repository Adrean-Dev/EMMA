#include <meca.h>
#include <logger.h>


struct Health {
    int value = 100;
    meca::entityID id;
};

struct Position {
    int x = 0;
    int y = 0;
    meca::entityID id;
};


int main() {
    auto start_creating = TimeShot();

    meca::componentRegistry<Health> healths;
    meca::componentRegistry<Position> positions;

    for(int i = 0; i < 10000; i++) {
        meca::create_entity();
        meca::create_component(i, {}, healths);
        meca::create_component(i, {}, positions);
    }

    auto end_creating = TimeShot();

    Log(INFO, "MAIN", elapsed_time(start_creating, end_creating, MICRO_SECONDS)); //Between 5000-7500 microseconds


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

    Log(INFO, "MAIN", elapsed_time(start_system, end_system, MICRO_SECONDS)); //Between 400-800 microseconds
}