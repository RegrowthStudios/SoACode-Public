///
/// IDGenerator.h
/// Vorb Engine
///
/// Created by Cristian Zaloj on 30 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Unique ID generator and recycler
///

#pragma once

#ifndef IDGenerator_h__
#define IDGenerator_h__

namespace vorb {
    namespace core {
#define ID_GENERATOR_NULL_ID 0

        /// Generates and recycles unique IDs of a certain type
        /// @template T: must support operator size_t() and operator++()
        template<typename T>
        class IDGenerator {
        public:
            /// Grabs an unique ID from either generation of recycling
            /// @param wasNew: Additional return value to determine if a new ID was created
            /// @return An unique ID
            T generate(bool* wasNew = nullptr) {
                T v;
                if (_recycled.size() > 0) {
                    v = _recycled.front();
                    _recycled.pop();
                    if (wasNew) *wasNew = false;
                } else {
                    _currentID++;
                    v = _currentID;
                    if (wasNew) *wasNew = true;
                }
                return v;
            }
            /// Returns an ID to a recycle queue
            /// @param v: ID to recycle
            void recycle(T& v) {
                _recycled.push(v);
            }

            /// @return Number of active IDs
            size_t getActiveCount() const {
                return static_cast<size_t>(_currentID) - _recycled.size();
            }
        private:
            T _currentID = ID_GENERATOR_NULL_ID; ///< Auto-incremented ID
            std::queue<T> _recycled; ///< List of recycled IDs
        };
    }
}
namespace vcore = vorb::core;

#endif // IDGenerator_h__