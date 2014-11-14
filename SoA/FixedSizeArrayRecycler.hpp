///
/// FixedSizeArrayRecycler.hpp
/// Vorb Engine
///
/// Created by Benjamin Arnold on 14 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// This class will recycle fixed sized arrays of a specific type.
/// This is very useful for chunk data but may have other uses.

#pragma once

#ifndef FixedSizeArrayRecycler_h__
#define FixedSizeArrayRecycler_h__

#include <vector>
#include <mutex>

namespace vorb {
    namespace core {

        template<int N, typename T>
        class FixedSizeArrayRecycler {
        public:
            /// Constructor
            /// @param maxSize: The maximum number of arrays to hold at any time.
            FixedSizeArrayRecycler(ui32 maxSize) : _maxSize(maxSize) { /* Empty */ }
            ~FixedSizeArrayRecycler() { destroy(); }

            /// Frees all arrays
            void destroy() {
                for (auto& data : _arrays) {
                    delete[] data;
                }
                std::vector<T*>().swap(_arrays);
            }

            /// Gets an array of type T with size N. May allocate new memory
            /// @return Pointer to the array
            T* create() {
                T* rv;
                if (_arrays.size()) {
                    rv = _arrays.back();
                    _arrays.pop_back();
                } else {
                    rv = new T[N];
                }
                return rv;
            }

            /// Recycles an array of type T with size N.
            /// Does not check for double recycles, and does not
            /// check that size is actually N, so be careful.
            /// @param data: The pointer to the array to be recycled.
            /// must be of size N. You should no longer use data after it has
            /// been recycled, as it may be deleted.
            void recycle(T* data) {
                /// Only recycle it if there is room for it
                if (_arrays.size() < _maxSize) {
                    _arrays.push_back(data);
                } else {
                    delete[] data;
                }
            }

            /// getters
            const size_t& getSize() const { return _arrays.size(); }
            int getArraySize() const { return N; }
            int getElementSize() const { return sizeof(T); }
        private:
            ui32 _maxSize; ///< Maximum number of arrays to hold
            std::vector<T*> _arrays; ///< Stack of recycled array data
        };
    }
}
// Namespace Alias
namespace vcore = vorb::core;

#endif // FixedSizeArrayRecycler_h__