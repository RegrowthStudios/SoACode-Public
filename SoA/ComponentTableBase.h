///
/// ComponentTableBase.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 9 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// 
///

#pragma once

#ifndef ComponentTableBase_h__
#define ComponentTableBase_h__

#include "Entity.h"

namespace vorb {
    namespace core {
        template<typename SortedSet1, typename SortedSet2>
        void commonEntities(std::vector<ui64>& rv, SortedSet1* c1, SortedSet2* c2);

        void commonEntities(std::vector<ui64>& rv, i32 count, ...);

        class ComponentTableBase {
        public:
            void onEntityRemoval(void* sender, ui64 id) {
                remove(id);
            }

            void add(ui64 eID);
            virtual void addComponent(ui64 eID) = 0;
            bool remove(ui64 eID);
            virtual void removeComponent(ui64 eID) = 0;

            const ui64* beginPtr() const {
                return &(_entities[0]);
            }
            const ui64* endPtr() const {
                return &(_entities[_entities.size() - 1]);
            }

            EntityIndexSet::const_iterator begin() const {
                return _entities.begin();
            }
            EntityIndexSet::const_iterator end() const {
                return _entities.end();
            }
            EntityIndexSet::const_iterator cbegin() const {
                return _entities.cbegin();
            }
            EntityIndexSet::const_iterator cend() const {
                return _entities.cend();
            }

            i32 getEntityCount() const {
                return _entities.size();
            }
        private:
            EntityIndexSet _entities;
        };

        template<i32 N>
        bool canIncrement(const size_t* indices, const i32* tableSizes) {
            for (i32 i = 0; i < N; i++) {
                if (indices[i] < tableSizes[i]) return true;
            }
            return false;
        }
        template<i32 N, typename T>
        bool areEqual(const size_t* indices, T* tables[N]) {
            for (i32 i = 1; i < N; i++) {
                if (tables[0][indices[0]] != tables[i][indices[i]]) return false;
            }
            return true;
        }
        template<typename T>
        bool loopUntilEqual(size_t& i1, const i32& c1, T* d1, size_t& i2, const i32& c2, T* d2) {
            while (i1 < c1 || i2 < c2) {
                if (i1 < c1) {
                    if (i2 < c2) {
                        if (d1[i1] < d2[i2]) i1++;
                        else if (d1[i1] > d2[i2]) i2++;
                        else return true;
                    } else {
                        if (d1[i1] < d2[i2]) i1++;
                        else return d1[i1] == d2[i2];
                    }
                } else {
                    if (d1[i1] > d2[i2]) i2++;
                    else return d1[i1] == d2[i2];
                }
            }
            return false;
        }
        template<i32 N, typename T>
        void forIntersection(void(*f)(size_t inds[N]), ...) {
            size_t indices[N];
            T* tables[N];
            i32 tableSizes[N];

            va_list args;
            va_start(args, f);
            for (i32 i = 0; i < N; i++) {
                indices[i] = 0;
                tables[i] = va_arg(args, T*);
                T* tend = va_arg(args, T*);
                tableSizes[i] = tend - tables[i];
            }
            va_end(args);

            while (canIncrement<N>(indices, tableSizes)) {
                for (i32 l = 1; l < N; l++) {
                    loopUntilEqual(indices[0], tableSizes[0], tables[0], indices[l], tableSizes[l], tables[l]) ? 1 : 0;
                }
                if (areEqual<N, T>(indices, tables)) {
                    f(indices);
                    for (i32 i = 0; i < N; i++) indices[i]++;
                }
            }
        }
    }
}
namespace vcore = vorb::core;

#endif // ComponentTableBase_h__

