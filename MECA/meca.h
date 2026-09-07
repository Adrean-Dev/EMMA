#pragma once
#define MAX_COMPONENTS 16




#include <vector>
#include <tuple>
#include <utility>
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
            public:
            std::vector<size_t> sparse;
            std::vector<T> dense;
            std::vector<size_t> compact;
            SparseSet() = default;

            void reserve(size_t size) {
                sparse.reserve(size);
                dense.reserve(size);
                compact.reserve(size);
                sparse.resize(size, -1);
            }

            inline void insert(size_t index, T&& element) {
                if((index >= sparse.size()) || (sparse[index] == -1)) {
                    if(index >= sparse.size()) sparse.resize(index+1, -1);
                    sparse[index] = dense.size();
                    dense.push_back(std::move(element));
                    compact.push_back(std::move(index));
                } else {
                    dense[sparse[index]] = element;
                }
            }

            inline void del(size_t index) {
                if((index < sparse.size()) && (sparse[index] != -1)) {
                    dense[sparse[index]] = dense.back();
                    sparse[compact.back()] = sparse[index];
                    compact[sparse[index]] = compact.back();
                    sparse[index] = -1;
                    dense.pop_back();
                    compact.pop_back();
                }
            }

            T* search(size_t index) {
                if((index < sparse.size()) && (sparse[index] != -1)) {
                    return &dense[sparse[index]];
                } else return nullptr;
            }

            inline bool has(size_t index) {
                return ((index < sparse.size()) && (sparse[index] != -1));
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

        size_t entity_count = 0;
        int componentType_count = 0;
    }


    template<typename T>
    class componentRegistry : public __internal::SparseSet<T> {
        public:
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
        __internal::entity_count++;
        return __internal::entity_count-1;
    }



    /*
    ####################
    COMPONENTS FUNCTIONS
    ####################
    */

    //Creates a new component for an entity.
    template<typename T>
    inline void create_component(entityID id, T&& component, componentRegistry<T> &registry) {
        if(__internal::entity_count > id) {
            //Inserting Component
            registry.insert(id, std::forward<T>(component));

            //Bits
            std::bitset<MAX_COMPONENTS> *mask = __internal::bitmasks.search(id);
            if(mask == nullptr) {
                __internal::bitmasks.insert(id, (0b1<<registry.component_id));
            } else {
                mask->set(registry.component_id);
            }
        } else Logger(LOGGER_WARNING, MECA_SYS, "Fail creating component!");
    }


    /*
    Gets the reference to an entity's component.
    @returns An entity component or nullptr (if component not found).
    */
    template<typename T>
    T* get_component(entityID id, componentRegistry<T> &registry) {
        T *component = registry.search(id);
        if(component == nullptr) Logger(LOGGER_WARNING, MECA_SYS, "Component search failed! The component doesn't exists.");
        return component;
    }


    //Deletes a component from an entity.
    template<typename T>
    void delete_component(entityID id, componentRegistry<T> &registry) {
        if(__internal::entity_count > id) {
            //Deleting Component
            registry.del(id);

            //Bits
            std::bitset<MAX_COMPONENTS> *mask = __internal::bitmasks.search(id);
            if(mask != nullptr) {
                mask->reset(registry.component_id);
            }
        } else Logger(LOGGER_WARNING, MECA_SYS, "Fail deleting component!");
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
    @param registries: All the component registries that you want to iterate.
    */
    template<typename... Registries, typename F>
    void filter_for(filter filtro, F &&function, Registries&... registries) {
        auto regs = std::forward_as_tuple(registries...);

        //Getting the minimum register
        size_t min = -1;
        entityID *min_id = 0;
        auto find_minimun = [&](auto& r) {
            if(r.dense.size() < min) {
                min = r.dense.size();
                min_id = r.compact.data();
            }
        };
        std::apply([&](auto&... reg) {(find_minimun(reg), ...);}, regs);

        //Actual system iteration
        for(size_t i = 0; i < min; i++) {
            bool has_all = (registries.has(min_id[i]) && ...);
            switch(filtro) {
                case AND_E:
                {
                    size_t mask = ((0b1 << registries.component_id) | ...);
                    std::bitset<MAX_COMPONENTS> *bitmask = __internal::bitmasks.search(min_id[i]);
                    if((bitmask != nullptr) && (bitmask->to_ullong() == mask)) {
                        function(registries.dense[registries.sparse[min_id[i]]]...);
                    }
                }
                break;
                case AND_I:
                if(has_all) {
                    function(registries.dense[registries.sparse[min_id[i]]]...);
                }
            }
        }
    }
}
