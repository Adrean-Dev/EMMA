#pragma once
#define MAX_COMPONENTS 16




#include <vector>
#include <bitset>

#if __has_include("logger.h")
#include <logger.h>
#else
#define Log(level, sys, msg)
#define INFO
#define WARNING
#define ERROR
#define FATAL
#endif

//Logger sys
#define MECA_SYS "MECA"
namespace meca {
    using entityID = size_t;


    namespace __internal {
        struct bitelement {
            std::bitset<MAX_COMPONENTS> bitmask;
            size_t index;
        };

        std::vector<bitelement> bitmasks;
        std::vector<std::vector<entityID>> bitgroups;

        int componentType_count = 0;
    }


    template<typename T>
    struct componentRegistry {
        std::vector<entityID> sparse;
        std::vector<T> dense;
        int component_id = -1;

        componentRegistry() {
            if(__internal::componentType_count < MAX_COMPONENTS) component_id = __internal::componentType_count, __internal::componentType_count++;
        }
    };

    enum filter {
        AND,
        OR
    };



    /*
    ################
    ENTITY FUNCTIONS
    ################
    */

    //Pre-allocates memory for entities.
    #define Reserve_entity_cap(capacity) __internal::bitmasks.reserve(capacity)


    /*
    Creates a new entity in a free id.
    @returns An id for new entity.
    */
    entityID create_entity() {
        __internal::bitmasks.push_back({0});
        return __internal::bitmasks.size()-1;
    }



    /*
    ####################
    COMPONENTS FUNCTIONS
    ####################
    */

    //Creates a new component for an entity.
    template<typename T>
    void create_component(entityID id, T component, componentRegistry<T> &registry) {
        if(!__internal::bitmasks.at(id).bitmask.test(registry.component_id)) {
            //Creating Component
            if(id >= registry.sparse.size()) registry.sparse.resize(id+1);
            registry.sparse.at(id) = registry.dense.size();
            component.id = id;
            registry.dense.push_back(component);

            //Bits
            if(__internal::bitmasks.at(id).bitmask.any()) {
                __internal::bitelement old = __internal::bitmasks.at(id);
                __internal::bitgroups.at(old.bitmask.to_ulong()).at(old.index) = __internal::bitgroups.at(old.bitmask.to_ulong()).back();
                __internal::bitgroups.at(old.bitmask.to_ulong()).pop_back();
                if(__internal::bitgroups.at(old.bitmask.to_ulong()).size() > old.index) __internal::bitmasks.at(__internal::bitgroups.at(old.bitmask.to_ulong()).at(old.index)).index = old.index;
            } //Swap & pop
            __internal::bitmasks.at(id).bitmask.set(registry.component_id);
            if(__internal::bitgroups.size() <= __internal::bitmasks.at(id).bitmask.to_ulong()) __internal::bitgroups.resize(__internal::bitmasks.at(id).bitmask.to_ulong()+1);
            __internal::bitmasks.at(id).index = __internal::bitgroups.at(__internal::bitmasks.at(id).bitmask.to_ulong()).size();
            __internal::bitgroups.at(__internal::bitmasks.at(id).bitmask.to_ulong()).push_back(id);
        } else Log(WARNING, MECA_SYS, "Fail creating component! The component already exists.");
    }


    /*
    Gets the reference to an entity's component.
    @returns An entity component or nullptr (if component not found).
    */
    template<typename T>
    T* get_component(entityID id, componentRegistry<T> &registry) {
        if(__internal::bitmasks.at(id).bitmask.test(registry.component_id)) {
            return &registry.dense.at(registry.sparse.at(id));
        } else {
            Log(WARNING, MECA_SYS, "Component search failed! The component doesn't exists.");
            return nullptr;
        }
    }


    //Deletes a component from an entity.
    template<typename T>
    void delete_component(entityID id, componentRegistry<T> &registry) {
        if(__internal::bitmasks.at(id).bitmask.test(registry.component_id)) {
            //Deleting Component
            registry.dense.at(registry.sparse.at(id)) = registry.dense.back();
            registry.sparse.at(registry.dense.back().id) = registry.sparse.at(id);
            registry.dense.pop_back();

            //Bits
            __internal::bitelement old = __internal::bitmasks.at(id);
            __internal::bitgroups.at(old.bitmask.to_ulong()).at(old.index) = __internal::bitgroups.at(old.bitmask.to_ulong()).back();
            __internal::bitgroups.at(old.bitmask.to_ulong()).pop_back();
            if(__internal::bitgroups.at(old.bitmask.to_ulong()).size() > old.index) __internal::bitmasks.at(__internal::bitgroups.at(old.bitmask.to_ulong()).at(old.index)).index = old.index;
            //Swap & pop

            __internal::bitmasks.at(id).bitmask.reset(registry.component_id);
            __internal::bitmasks.at(id).index = __internal::bitgroups.at(__internal::bitmasks.at(id).bitmask.to_ulong()).size();
            __internal::bitgroups.at(__internal::bitmasks.at(id).bitmask.to_ulong()).push_back(id);
        } else Log(WARNING, MECA_SYS, "Fail deleting component! The component doesn't exists.");
    }


    //Gives a simple iterator (std::vector) of components (references to component registry).
    template<typename T>
    std::vector<T>& component_iterator(componentRegistry<T> &registry) {
        return registry.dense;
    }


    /*
    Alternative to for: it gives support for multiple component registry iteration.
    @param filtro: It's the type of filtering wanted for getting the components, indicated by logical gates.
    @param function: A function (it can be lambda) that operates with the components needed.
    @param registries: All the component registries that you want to iterate (AND looping).
    */
    template<typename... Registries, typename F>
    void filter_for(filter filtro, F &&function, Registries&... registries) {
        size_t mask = ((0b1 << registries.component_id) | ...);
        switch(filtro) {
            case AND:
            for(entityID &id : __internal::bitgroups.at(mask)) {
                function(registries.dense.at(registries.sparse.at(id))...);
            }
            break;
            case OR:
            std::bitset<MAX_COMPONENTS> bitmask = mask;
            for(size_t i = mask; (i < __internal::bitgroups.size()) && (bitmask.test(registries.component_id) && ...); i++) {
                for(entityID &id : __internal::bitgroups.at(i)) {
                    function(registries.dense.at(registries.sparse.at(id))...);
                }
            }
        }
    }
}
