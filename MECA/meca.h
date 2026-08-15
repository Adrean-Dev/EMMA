#pragma once
#define MAX_COMPONENTS 16




#include <vector>
#include <unordered_set>
#include <bitset>

#if __has_include("logger.h")
#include <logger.h>
#else
#define Logger(level, sys, msg)
#define LOGGER_INFO
#define LOGGER_WARNING
#define LOGGER_ERROR
#define LOGGER_FATAL
#endif

//Logger sys
#define MECA_SYS "MECA"
namespace meca {
    using entityID = size_t;


    namespace __internal {

        template<typename T>
        class SparseSet {
            private:
            std::vector<size_t> compact;

            public:
            std::vector<size_t> sparse;
            std::vector<T> dense;
            SparseSet() = default;
            SparseSet(size_t reserve_n) {
                sparse.reserve(reserve_n);
                dense.reserve(reserve_n);
                compact.reserve(reserve_n);
            }

            void insert(size_t index, T element) {
                if(index >= sparse.size()) sparse.resize(index+1, -1);
                sparse[index] = dense.size();
                dense.push_back(element);
                compact.push_back(index);
            }

            void del(size_t index) {
                if((index < sparse.size()) && (sparse[index] < dense.size())) {
                    dense[sparse[index]] = dense.back();
                    sparse[compact.back()] = sparse[index];
                    sparse[index] = -1;
                    dense.pop_back();
                    compact.pop_back();
                }
            }

            T* search(size_t index) {
                if((index < sparse.size()) && (sparse[index] < dense.size())) {
                    return &dense[sparse[index]];
                } else return nullptr;
            }

            bool has(size_t index) {
                return ((index < sparse.size()) && (sparse[index] < dense.size())) ? true : false;
            }

            void clear() {
                sparse.clear();
                dense.clear();
                compact.clear();
                sparse.shrink_to_fit();
                dense.shrink_to_fit();
                compact.shrink_to_fit();
            }
        };

        SparseSet<std::bitset<MAX_COMPONENTS>> bitmasks;
        SparseSet<std::unordered_set<entityID>> bitgroups;

        int componentType_count = 0;
    }


    template<typename T>
    struct componentRegistry {
        __internal::SparseSet<T> sparse_set;
        int component_id = -1;

        componentRegistry() {
            if(__internal::componentType_count < MAX_COMPONENTS) component_id = __internal::componentType_count, __internal::componentType_count++;
        }
    };

    enum filter {
        AND_E,
        AND_I
    };



    /*
    ################
    ENTITY FUNCTIONS
    ################
    */

    /* RE-DO
    Creates a new entity in a free id.
    @returns An id for new entity.
    */
    entityID create_entity() {
        size_t id = __internal::bitmasks.dense.size();
        __internal::bitmasks.insert(id, 0);
        if(!__internal::bitgroups.has(0)) __internal::bitgroups.insert(0, {});
        __internal::bitgroups.search(0)->insert(id);
        return id;
    }



    /*
    ####################
    COMPONENTS FUNCTIONS
    ####################
    */

    //Creates a new component for an entity.
    template<typename T>
    void create_component(entityID id, T component, componentRegistry<T> &registry) {
        if(!registry.sparse_set.has(id) && (__internal::bitmasks.dense.size() > id)) {
            //Inserting Component
            registry.sparse_set.insert(id, component);

            //Bits
            std::bitset<MAX_COMPONENTS> *mask = __internal::bitmasks.search(id);
            __internal::bitgroups.search(mask->to_ullong())->erase(id);
            mask->set(registry.component_id);
            if(!__internal::bitgroups.has(mask->to_ullong())) __internal::bitgroups.insert(mask->to_ullong(), {});
            __internal::bitgroups.search(mask->to_ullong())->insert(id);
        } else Logger(LOGGER_WARNING, MECA_SYS, "Fail creating component!");
    }


    /*
    Gets the reference to an entity's component.
    @returns An entity component or nullptr (if component not found).
    */
    template<typename T>
    T* get_component(entityID id, componentRegistry<T> &registry) {
        T *component = registry.sparse_set.search(id);
        if(component == nullptr) Logger(LOGGER_WARNING, MECA_SYS, "Component search failed! The component doesn't exists.");
        return component;
    }


    //Deletes a component from an entity.
    template<typename T>
    void delete_component(entityID id, componentRegistry<T> &registry) {
        if(registry.sparse_set.has(id) && (__internal::bitmasks.dense.size() > id)) {
            //Deleting Component
            registry.sparse_set.del(id);

            //Bits
            std::bitset<MAX_COMPONENTS> *mask = __internal::bitmasks.search(id);
            __internal::bitgroups.search(mask->to_ullong())->erase(id);
            mask->reset(registry.component_id);
            //if(!__internal::bitgroups.has(mask->to_ullong())) __internal::bitgroups.insert(mask->to_ullong(), {});
            __internal::bitgroups.search(mask->to_ullong())->insert(id);
        } else Logger(LOGGER_WARNING, MECA_SYS, "Fail deleting component!");
    }


    //Gives a simple iterator (std::vector) of components (references to component registry).
    template<typename T>
    std::vector<T>& component_iterator(componentRegistry<T> &registry) {
        return registry.sparse_set.dense;
    }


    /*
    Alternative to for: it gives support for multiple component registry iteration.
    @param filtro: It's the type of filtering wanted for getting the components, indicated by logical gates.
    @param function: A function (it can be lambda) that operates with the components needed.
    @param registries: All the component registries that you want to iterate.
    */
    template<typename... Registries, typename F>
    void filter_for(filter filtro, F &&function, Registries&... registries) {
        size_t mask = ((0b1 << registries.component_id) | ...);
        switch(filtro) {
            case AND_E:
            for(entityID id : *__internal::bitgroups.search(mask)) {
                function(*registries.sparse_set.search(id)...);
            }
            break;
            case AND_I:
            std::bitset<MAX_COMPONENTS> bitmask = mask;
            for(size_t i = mask; i < __internal::bitgroups.sparse.size(); i = (i+1)|mask) {
                for(entityID id : *__internal::bitgroups.search(i)) {
                    function(*registries.sparse_set.search(id)...);
                }
            }
        }
    }
}
