#pragma once
#define MAX_COMPONENTS 16




#include <assert.h>
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

            inline T& get(entityID id) {
                return dense[sparse[id]];
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

        //OFU flags (Only First used)
        template<typename T>
        inline bool type_used = false;


        //This gets a unique id from a datatype
        template<typename T>
        size_t get_componentType_id() {
            static size_t id = componentType_count++;
            return id;
        }

        //This gets a raw component from an entity and componentType (without handling non-existance)
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
    Creates a new entity.
    @returns An id for new entity.
    */
    inline entityID create_entity() {
        return __internal::entity_count++;
    }


    //Resets an entity (deletes all its components).
    inline void reset_entity(entityID id) {
        assert((id < __internal::entity_count) && (id < __internal::bitmasks.size()) && (__internal::bitmasks[id].any()));
        for(size_t i = 0; i < MAX_COMPONENTS; i++) {
            if(__internal::bitmasks[id].test(i)) {
                __internal::registry[i].get()->del(id);
            }
        }
        __internal::bitmasks[id].reset();
    }



    /*
    ####################
    COMPONENTS FUNCTIONS
    ####################
    */
    
    //Preallocates memory for a component type (only use it before creating any component of that type)
    template<typename T>
    void reserve_components(size_t size, T default_val) {
        //OFU Flag
        if(__internal::type_used<T>) return;
        __internal::type_used<T> = true;

        size_t id = __internal::get_componentType_id<T>();
        if(id >= __internal::registry.size()) __internal::registry.push_back(std::make_unique<__internal::SparseRegistry<T>>());

        __internal::SparseRegistry<T> *reg = static_cast<__internal::SparseRegistry<T>*>(__internal::registry[id].get());

        reg->dense.resize(size, default_val);

        for(size_t i = 0; i < size; i++) {
            reg->sparse.push_back(i);
            reg->compact.push_back(i);
        }
    }


    //Creates a new component for an entity.
    template<typename T>
    inline void create_component(entityID id, T&& component) {
        assert(id < __internal::entity_count);
        
        //OFU Flag
        __internal::type_used<T> = true;

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
    }


    /*
    Gets an entity's component.
    @returns A reference to the entity component.
    */
    template<typename T>
    inline T& get_component(entityID id) {
        assert((id < __internal::entity_count) && (__internal::type_used<T>));
        return static_cast<__internal::SparseRegistry<T>*>(__internal::registry[__internal::get_componentType_id<T>()].get())->get(id);
    }


    //Deletes a component from an entity.
    template<typename T>
    inline void delete_component(entityID id) {
        assert((id < __internal::entity_count) && (__internal::type_used<T>));

        size_t reg_id = __internal::get_componentType_id<T>();
        __internal::SparseRegistry<T> *comp_reg = static_cast<__internal::SparseRegistry<T>*>(__internal::registry[reg_id].get());

        //Bits
        if(comp_reg->has(id)) __internal::bitmasks[id].reset(reg_id);

        //Registry
        comp_reg->del(id);
    }


    /*
    Gives a vector of components (from one component type).
    @returns A reference to the internal vector of a component registry.
    @note Use it in loops, trust me ;)
    */
    template<typename T>
    std::vector<T>& component_iterator() {
        assert(__internal::type_used<T>);

        __internal::SparseRegistry<T> *comp_reg = static_cast<__internal::SparseRegistry<T>*>(__internal::registry[__internal::get_componentType_id<T>()].get());

        assert(comp_reg->dense.size() > 0);

        return comp_reg->dense;
    }


    /*
    Alternative to for: it gives support for multiple component iteration.
    @param filtro: It's the type of filtering wanted for getting the components, indicated by logical gates.
    @param function: A function (it can be lambda) that operates with the components needed.
    */
    template<typename... Components, typename F>
    void filter_for(filter filtro, F &&function) {
        assert(__internal::type_used<Components> && ...);

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
