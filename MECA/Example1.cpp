#include <meca.h>
#include <logger.h> //I recommend using Logger for better debugging and output ;)
#include <string>
using std::string;
using namespace meca;

//Create structs for all your components c:
struct Position {
    int x = 0;
    int y = 0;
    entityID id; //Important!!! for all components struct it needs an entityID named id.
};

struct Health {
    int value = 100;
    entityID id;
};

struct Rank {
    char grade = 'Z';
    int points = 0;
    entityID id;
};


int main() {
    //Create registries for your components! It's a way of grouping some components together.
    componentRegistry<Position> positions;
    componentRegistry<Health> healths;
    componentRegistry<Rank> ranks;

    //Create your first entity B)
    entityID mahoma = create_entity();
    //Another nice guy
    entityID juan = create_entity();

    //Give your entities some components, registering them in your favorite componentRegistry.
    create_component(mahoma, {100,200}, positions);
    create_component(mahoma, {999}, healths);

    create_component(juan, {5, 10}, positions);
    create_component(juan, {20}, healths);
    create_component(juan, {'A', 1000}, ranks);

    //Use your components in straightforward ways!!!

    //Use filter_for to loop over more than one component registry.
    filter_for(
        AND_I,
        [](Position &pos, Health &hp) {
            //Output some info with Logger!
            Log(INFO, MECA_SYS, "This character has a position of ("<<pos.x<<", "<<pos.y<<") and "<<hp.value<<" of HP.");
        },
        positions, healths
    );

    //Use your range based for to loop over one component registry.
    Log(INFO, MECA_SYS, "Here is the list of points gained in the match:");
    for(Rank &rank : component_iterator(ranks)) {
        Log(INFO, MECA_SYS, rank.points);
    }
}