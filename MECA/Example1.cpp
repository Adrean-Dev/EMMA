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


int main() {
    //Create registries for your components! It's a way of grouping some components together.
    componentRegistry<Position> positions;
    componentRegistry<Health> healths;

    //Create your first entity B)
    entityID mahoma = create_entity();

    //Give your entity some components, registering them in your favorite componentRegistry.
    create_component(mahoma, {100,200}, positions);
    create_component(mahoma, {999}, healths);

    //Use your components in straightforward ways!!!
    filter_for(
        [](Position &pos, Health &hp) {
            //Output some info with Logger!
            Log(INFO, MECA_SYS, "Mahoma has a position of ("<<pos.x<<", "<<pos.y<<") and "<<hp.value<<" of HP.");
        }, positions, healths
    );
}