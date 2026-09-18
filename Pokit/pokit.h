#pragma once

#include <vector>
#include <utility>


namespace pokit {
    //SparseSet
    template<typename T>
    class SparseSet {
        private:
        std::vector<size_t> sparse;
        std::vector<T> dense;
        std::vector<size_t> compact;

        public:
        SparseSet() = default;

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
}