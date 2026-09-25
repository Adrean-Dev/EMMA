#include <meca.h>
#include <logger.h> //I recommend using Logger for better debugging and output ;)
#include <string>
using std::string;
using namespace meca;

//Create structs for all your components c:
struct Position {
    int x = 0;
    int y = 0;
};

struct Health {
    int value = 100;
};

struct Rank {
    char grade = 'Z';
    int points = 0;
};


int main() {
    //Create your first entity B)
    entityID mahoma = create_entity();
    //Another nice guy
    entityID juan = create_entity();

    //Give your entities some components!
    create_component<Position>(mahoma, {100,200});
    create_component<Health>(mahoma, {999});

    create_component<Position>(juan, {5, 10});
    create_component<Health>(juan, {20});
    create_component<Rank>(juan, {'A', 1000});

    //Use your components in straightforward ways!!!

    //Use filter_for to loop over more than one component type.
    filter_for<Position, Health>(
        AND_Included,
        [](Position &pos, Health &hp) {
            //Output some info with Logger!
            Logger(LOGGER_INFO, MECA_SYS, "This character has a position of ("<<pos.x<<", "<<pos.y<<") and "<<hp.value<<" of HP.");
        }
    );

    //Use your range based for to loop over one component type.
    Logger(LOGGER_INFO, MECA_SYS, "Here is the list of points gained in the match:");
    for(Rank &rank : component_iterator<Rank>()) {
        Logger(LOGGER_INFO, MECA_SYS, rank.points);
    }
}