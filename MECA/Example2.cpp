#include <meca.h>
#include <logger.h>

int main() {
    #define entities 10000
    #define iterations 100

    //Creating entities
    int total_creating = 0;
    for(int a = 0; a < iterations; a++) {
        auto start_creating = TimeShot();

        for(int i = 0; i < entities; i++) {
            meca::create_entity();
        }

        auto end_creating = TimeShot();

        total_creating += elapsed_time(start_creating, end_creating, LOGGER_MICRO_SECONDS);
    }
    total_creating /= iterations;

    Logger(LOGGER_INFO, "MAIN", "create_entity: " << total_creating);



    for(int i = 0; i < entities; i++) {
        meca::create_component<int>(i, 7);
    }



    
    int total_del = 0;
    int total_comps = 0;
    for(int a = 0; a < iterations; a++) {
        //Deleting components
        auto start_del = TimeShot();

        for(int i = 0; i < entities; i++) {
            meca::delete_component<int>(i);
        }

        auto end_del = TimeShot();

        total_del += elapsed_time(start_del, end_del, LOGGER_MICRO_SECONDS);

        //Creating components
        auto start_comps = TimeShot();

        for(int i = 0; i < entities; i++) {
            meca::create_component<int>(i, 7);
        }

        auto end_comps = TimeShot();

        total_comps += elapsed_time(start_comps, end_comps, LOGGER_MICRO_SECONDS);
    }
    total_comps /= iterations;
    total_del /= iterations;

    Logger(LOGGER_INFO, "MAIN", "create_component: " << total_comps);
    Logger(LOGGER_INFO, "MAIN", "delete_component: " << total_del);



    //Getting components
    int total_get = 0;
    for(int a = 0; a < iterations; a++) {
        auto start_get = TimeShot();

        for(int i = 0; i < entities; i++) {
            int *x = meca::get_component<int>(i);
        }

        auto end_get = TimeShot();

        total_get += elapsed_time(start_get, end_get, LOGGER_MICRO_SECONDS);
    }
    total_get /= iterations;

    Logger(LOGGER_INFO, "MAIN", "get_component: " << total_get);



    //Iterating components
    int total_iter = 0;
    for(int a = 0; a < iterations; a++) {
        auto start_iter = TimeShot();

        for(int &element : *meca::component_iterator<int>()) {
            element++;
        }

        auto end_iter = TimeShot();

        total_iter += elapsed_time(start_iter, end_iter, LOGGER_MICRO_SECONDS);
    }
    total_iter /= iterations;

    Logger(LOGGER_INFO, "MAIN", "component_iterator: " << total_iter);

    
    
    for(int i = 0; i < entities; i++) {
        meca::create_component<char>(i, 'A');
    }



    //Using filter_for
    int total_for = 0;
    for(int a = 0; a < iterations; a++) {
        auto start_for = TimeShot();

        meca::filter_for<int, char>(meca::AND_Included, 
            [&](int &a, char &b) {
                if(b == 'A') a++;
            }
        );

        auto end_for = TimeShot();

        total_for += elapsed_time(start_for, end_for, LOGGER_MICRO_SECONDS);
    }
    total_for /= iterations;

    Logger(LOGGER_INFO, "MAIN", "filter_for (2 components): " << total_for);



    int total_reset = 0;
    for(int a = 0; a < iterations; a++) {
        //Resetting entities
        auto start_reset = TimeShot();

        for(int i = 0; i < entities; i++) {
            meca::reset_entity(i);
        }

        auto end_reset = TimeShot();

        total_reset += elapsed_time(start_reset, end_reset, LOGGER_MICRO_SECONDS);

        //Creating components
        for(int i = 0; i < entities; i++) {
            meca::create_component<int>(i, 7);
        }
    }
    total_reset /= iterations;

    Logger(LOGGER_INFO, "MAIN", "reset_entity: " << total_reset);
}