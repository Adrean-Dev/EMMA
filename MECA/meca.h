#pragma once
#define MAX_COMPONENTS 16




#include <memory>
#include <vector>
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
        class ISparseRegistry {
            public:
            size_t dense_size = 0;

            virtual ~ISparseRegistry() = default;
            virtual void del(entityID id) = 0;
        };

        template<typename T>
        class SparseRegistry : public ISparseRegistry {
            public:
            std::vector<entityID> sparse;
            std::vector<T> dense;
            std::vector<entityID> compact;

            inline void insert(entityID id, T&& component) {
                if(!has(id)) {
                    if(id >= sparse.size()) sparse.resize(id+1, -1);
                    sparse[id] = dense.size();
                    dense.push_back(std::move(component));
                    compact.push_back(std::move(id));
                    dense_size++;
                } else {
                    dense[sparse[id]] = component;
                }
            }

            void del(entityID id) override {
                if(has(id)) {
                    dense[sparse[id]] = dense.back();
                    sparse[compact.back()] = sparse[id];
                    compact[sparse[id]] = compact.back();
                    sparse[id] = -1;
                    dense.pop_back();
                    compact.pop_back();
                    dense_size--;
                }
            }

            T* get(entityID id) {
                return (has(id)) ? &dense[sparse[id]] : nullptr;
            }

            constexpr bool has(entityID id) {
                return ((id < sparse.size()) && (sparse[id] != -1));
            }
        };


        //Components data structures
        std::vector<std::unique_ptr<ISparseRegistry>> registry;
        std::vector<std::bitset<MAX_COMPONENTS>> bitmasks;

        //Counts
        size_t entity_count = 0;
        size_t componentType_count = 0;

        //This gets a unique id from a datatype
        template<typename T>
        size_t get_componentType_id() {
            static size_t id = componentType_count++;
            return id;
        }

        //This gets a raw component from a type (without handling non-existance)
        template<typename T>
        T& get_component_byType(entityID id) {
            SparseRegistry<T> *comp_reg = static_cast<SparseRegistry<T>*>(registry[get_componentType_id<T>()].get());
            return comp_reg->dense[comp_reg->sparse[id]];
        }
    }


    enum filter {
        AND_Excluded,
        AND_Included
    };



    /*
    ################
    ENTITY FUNCTIONS
    ################
    */

    /*
    Creates a new entity in a free id.
    @returns An id for new entity.
    */
    entityID create_entity() {
        __internal::entity_count++;
        return __internal::entity_count-1;
    }


    //Resets an entity (deletes all its components).
    inline void reset_entity(entityID id) {
        if((id < __internal::entity_count) && (id < __internal::bitmasks.size()) && (__internal::bitmasks[id].any())) {
            for(size_t i = 0; i < MAX_COMPONENTS; i++) {
                if(__internal::bitmasks[id].test(i)) {
                    __internal::registry[i].get()->del(id);
                }
            }
            __internal::bitmasks[id].reset();
        }
    }



    /*
    ####################
    COMPONENTS FUNCTIONS
    ####################
    */

    //Creates a new component for an entity.
    template<typename T>
    inline void create_component(entityID id, T&& component) {
        if(id < __internal::entity_count) {
            size_t reg_id = __internal::get_componentType_id<T>();
            if(reg_id >= __internal::registry.size()) __internal::registry.push_back(std::make_unique<__internal::SparseRegistry<T>>());
            
            //Registry
            __internal::SparseRegistry<T> *comp_reg = static_cast<__internal::SparseRegistry<T>*>(__internal::registry[reg_id].get());
            comp_reg->insert(id, std::forward<T>(component));

            //Bits
            if(id >= __internal::bitmasks.size()) {
                __internal::bitmasks.resize(id+1, 0);
                __internal::bitmasks[id].set(reg_id);
            } else {
                __internal::bitmasks[id].set(reg_id);
            }
        } else Logger(LOGGER_WARNING, MECA_SYS, "Fail creating component! Invalid entityID: " << id);
    }


    /*
    Gets the pointer to an entity's component.
    @returns An entity component or nullptr (if component not found).
    */
    template<typename T>
    T* get_component(entityID id) {
        if(id >= __internal::entity_count) return nullptr;

        size_t reg_id = __internal::get_componentType_id<T>();
        __internal::SparseRegistry<T> *comp_reg = (reg_id < __internal::registry.size()) ? static_cast<__internal::SparseRegistry<T>*>(__internal::registry[reg_id].get()) : nullptr;
        if(comp_reg == nullptr) return nullptr;

        return comp_reg->get(id);
    }


    //Deletes a component from an entity.
    template<typename T>
    inline void delete_component(entityID id) {
        size_t reg_id = __internal::get_componentType_id<T>();
        if((id < __internal::entity_count) && (reg_id < __internal::registry.size())) {
            __internal::SparseRegistry<T> *comp_reg = static_cast<__internal::SparseRegistry<T>*>(__internal::registry[reg_id].get());

            //Bits
            if(comp_reg->has(id)) __internal::bitmasks[id].reset(reg_id);

            //Registry
            comp_reg->del(id);
        }  else Logger(LOGGER_WARNING, MECA_SYS, "Fail deleting component! Invalid component type or entityID: " << id);
    }


    /*
    Gives a simple pointer to a vector of components (references to component registry).
    @returns A pointer to the internal vector of a component registry or nullptr (if not found).
    */
    template<typename T>
    std::vector<T>* component_iterator() {
        size_t reg_id = __internal::get_componentType_id<T>();
        if(reg_id < __internal::registry.size()) {
            __internal::SparseRegistry<T> *comp_reg = static_cast<__internal::SparseRegistry<T>*>(__internal::registry[reg_id].get());
            return (comp_reg->dense.size() > 0) ? &comp_reg->dense : nullptr;
        }
        return nullptr;
    }


    /*
    Alternative to for: it gives support for multiple component iteration.
    @param filtro: It's the type of filtering wanted for getting the components, indicated by logical gates.
    @param function: A function (it can be lambda) that operates with the components needed.
    */
    template<typename... Components, typename F>
    void filter_for(filter filtro, F &&function) {
        //Getting the minimum component registry size
        size_t min_id = 0; //The good stuff

        size_t min = -1;
        (
            [&]() {
                size_t reg_id = __internal::get_componentType_id<Components>();
                if(__internal::registry[reg_id].get()->dense_size < min) {
                    min = __internal::registry[reg_id].get()->dense_size;
                    min_id = reg_id;
                }
            }(), ...
        );


        std::bitset<MAX_COMPONENTS> mask; //Bitmask of the components for this loop
        (mask.set(__internal::get_componentType_id<Components>()), ...);

        //This executes the function directly with the entity id
        auto execute_func = [&](entityID &id) {
            function(__internal::get_component_byType<Components>(id)...);
        };

        //Looping through the components using a condition
        (
            [&]() {
                if(__internal::get_componentType_id<Components>() == min_id) {
                    __internal::SparseRegistry<Components>* comp_reg = static_cast<__internal::SparseRegistry<Components>*>(__internal::registry[min_id].get());
                    for(entityID &id : comp_reg->compact) {
                        switch(filtro) {
                            case AND_Excluded:
                            if(__internal::bitmasks[id] == mask) {
                                execute_func(id);
                            }
                            break;
                            
                            case AND_Included:
                            if((__internal::bitmasks[id] & mask) == mask) {
                                execute_func(id);
                            }
                            break;
                        }
                    }
                }
            }(), ...
        );
    }
}
