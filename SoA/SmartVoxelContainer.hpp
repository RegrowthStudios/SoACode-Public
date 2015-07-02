//
// SmartVoxelContainer.h
// Vorb Engine
//
// Created by Benjamin Arnold on 14 Nov 2014
// Copyright 2014 Regrowth Studios
// All Rights Reserved
//
// Summary:
// 
//

#pragma once

#ifndef SmartVoxelContainer_h__
#define SmartVoxelContainer_h__

#include <mutex>

#include "Constants.h"

#include <Vorb/FixedSizeArrayRecycler.hpp>
#include <Vorb/Voxel/IntervalTree.h>

#define QUIET_FRAMES_UNTIL_COMPRESS 60
#define ACCESS_COUNT_UNTIL_DECOMPRESS 5

// TODO(Cristian): We'll see how to fit it into Vorb
namespace vorb {
    namespace voxel {

        static ui32 MAX_COMPRESSIONS_PER_FRAME = UINT_MAX; ///< You can optionally set this in order to limit changes per frame
        static ui32 totalContainerCompressions = 1; ///< Set this to 1 each frame

        template<typename T, size_t SIZE> class SmartVoxelContainer;

        template<typename T, size_t SIZE>
        class SmartHandle {
            friend class SmartVoxelContainer<T, SIZE>;
        public:
            operator const T&() const;

            SmartHandle& operator= (T data);
            SmartHandle& operator= (const SmartHandle& o);

            SmartHandle(SmartHandle&& o) :
                m_container(o.m_container),
                m_index(o.m_index) {
                // TODO: Empty for now try to null it out
            }
            SmartHandle(const SmartHandle& o) = delete;
            SmartHandle& operator= (SmartHandle&& o) = delete;
        private:
            SmartHandle(SmartVoxelContainer<T, SIZE>& container, size_t index) :
                m_container(container),
                m_index(index) {
                // Empty
            }

            SmartVoxelContainer<T, SIZE>& m_container; ///< The parent container that created the handle
            size_t m_index; ///< The index of this handle into the smart container
        };

        /// This should be called once per frame to reset totalContainerChanges
        inline void clearContainerCompressionsCounter() {
            totalContainerCompressions = 1; ///< Start at 1 so that integer overflow handles the default case
        }

        enum class VoxelStorageState {
            FLAT_ARRAY = 0,
            INTERVAL_TREE = 1
        };

        template <typename T, size_t SIZE = CHUNK_SIZE>
        class SmartVoxelContainer {
            friend class SmartHandle<T, SIZE>;
        public:
            /// Constructor
            SmartVoxelContainer() {
                // Empty
            }
            /*! @brief Construct the container with a provided recycler.
            *
            * @param arrayRecycler: The recycler to be used in place of the default generated recycler.
            */
            SmartVoxelContainer(vcore::FixedSizeArrayRecycler<SIZE, T>* arrayRecycler) {
                setArrayRecycler(arrayRecycler);
            }

            /*! @brief Change the array recycler.
            *
            * @param arrayRecycler: The recycler to be used in place of the default generated recycler.
            */
            void setArrayRecycler(vcore::FixedSizeArrayRecycler<SIZE, T>* arrayRecycler) {
                _arrayRecycler = arrayRecycler;
            }

            SmartHandle<T, SIZE> operator[] (size_t index) {
                return std::move(SmartHandle<T, SIZE>(*this, index));
            }
            const T& operator[] (size_t index) const {
                return (getters[(size_t)_state])(this, index);
            }

            /// Initializes the container
            inline void init(VoxelStorageState state) {
                _state = state;
                if (_state == VoxelStorageState::FLAT_ARRAY) {
                    _dataArray = _arrayRecycler->create();
                }
            }

            /// Creates the tree using a sorted array of data. 
            /// The number of voxels should add up to CHUNK_SIZE
            /// @param state: Initial state of the container
            /// @param data: The sorted array used to populate the container
            inline void initFromSortedArray(VoxelStorageState state,
                                            const std::vector <typename IntervalTree<T>::LNode>& data) {
                _state = state;
                _accessCount = 0;
                _quietFrames = 0;
                if (_state == VoxelStorageState::INTERVAL_TREE) {
                    _dataTree.initFromSortedArray(data);
                    _dataTree.checkTreeValidity();
                } else {
                    _dataArray = _arrayRecycler->create();
                    int index = 0;
                    for (int i = 0; i < data.size(); i++) {
                        for (int j = 0; j < data[i].length; j++) {
                            _dataArray[index++] = data[i].data;
                        }
                    }
                }
            }
            inline void initFromSortedArray(VoxelStorageState state,
                                            const typename IntervalTree<T>::LNode data[], size_t size) {
                _state = state;
                _accessCount = 0;
                _quietFrames = 0;
                if (_state == VoxelStorageState::INTERVAL_TREE) {
                    _dataTree.initFromSortedArray(data, size);
                    _dataTree.checkTreeValidity();
                } else {
                    _dataArray = _arrayRecycler->create();
                    int index = 0;
                    for (int i = 0; i < size; i++) {
                        for (int j = 0; j < data[i].length; j++) {
                            _dataArray[index++] = data[i].data;
                        }
                    }
                }
            }

            inline void changeState(VoxelStorageState newState, std::mutex& dataLock) {
                if (newState == _state) return;
                if (newState == VoxelStorageState::INTERVAL_TREE) {
                    compress(dataLock);
                } else {
                    uncompress(dataLock);
                }
                _quietFrames = 0;
                _accessCount = 0;
            }

            /// Updates the container. Call once per frame
            /// @param dataLock: The mutex that guards the data
            inline void update(std::mutex& dataLock) {
                // If access count is higher than the threshold, this is not a quiet frame
                if (_accessCount >= ACCESS_COUNT_UNTIL_DECOMPRESS) {
                    _quietFrames = 0;
                } else {
                    _quietFrames++;
                }

                if (_state == VoxelStorageState::INTERVAL_TREE) {
                    // Check if we should uncompress the data
                    if (_quietFrames == 0) {
                        uncompress(dataLock);
                    }
                } else {
                    // Check if we should compress the data
                    if (_quietFrames >= QUIET_FRAMES_UNTIL_COMPRESS && totalContainerCompressions <= MAX_COMPRESSIONS_PER_FRAME) {
                        compress(dataLock);
                    }
                }
                _accessCount = 0;
            }

            /// Clears the container and frees memory
            inline void clear() {
                _accessCount = 0;
                _quietFrames = 0;
                if (_state == VoxelStorageState::INTERVAL_TREE) {
                    _dataTree.clear();
                } else if (_dataArray) {
                    _arrayRecycler->recycle(_dataArray);
                    _dataArray = nullptr;
                }
            }

            /// Uncompressed the interval tree into a buffer.
            /// May only be called when getState() == VoxelStorageState::INTERVAL_TREE
            /// or you will get a null access violation.
            /// @param buffer: Buffer of memory to store the result
            inline void uncompressIntoBuffer(T* buffer) { _dataTree.uncompressIntoBuffer(buffer); }

            /// Getters
            const VoxelStorageState& getState() const {
                return _state;
            }
            T* getDataArray() {
                return _dataArray;
            }
            const T* getDataArray() const {
                return _dataArray;
            }
            IntervalTree<T>& getTree() {
                return _dataTree;
            }
            const IntervalTree<T>& getTree() const {
                return _dataTree;
            }

            /// Gets the element at index
            /// @param index: must be (0, SIZE]
            /// @return The element
            inline const T& get(size_t index) const {
                return (getters[(size_t)_state])(this, index);
            }
            /// Sets the element at index
            /// @param index: must be (0, SIZE]
            /// @param value: The value to set at index
            inline void set(size_t index, T value) {
                _accessCount++;
                (setters[(size_t)_state])(this, index, value);
            }
        private:
            typedef const T& (*Getter)(const SmartVoxelContainer*, size_t);
            typedef void(*Setter)(SmartVoxelContainer*, size_t, T);

            static const T& getInterval(const SmartVoxelContainer* container, size_t index) {
                return container->_dataTree.getData(index);
            }
            static const T& getFlat(const SmartVoxelContainer* container, size_t index) {
                return container->_dataArray[index];
            }
            static void setInterval(SmartVoxelContainer* container, size_t index, T data) {
                container->_dataTree.insert(index, data);
            }
            static void setFlat(SmartVoxelContainer* container, size_t index, T data) {
                container->_dataArray[index] = data;
            }

            static Getter getters[2];
            static Setter setters[2];

            inline void uncompress(std::mutex& dataLock) {
                dataLock.lock();
                _dataArray = _arrayRecycler->create();
                uncompressIntoBuffer(_dataArray);
                // Free memory
                _dataTree.clear();
                // Set the new state
                _state = VoxelStorageState::FLAT_ARRAY;
                dataLock.unlock();
            }
            inline void compress(std::mutex& dataLock) {
                dataLock.lock();
                // Sorted array for creating the interval tree
                // Using stack array to avoid allocations, beware stack overflow
                IntervalTree<T>::LNode data[CHUNK_SIZE];
                int index = 0;
                data[0].set(0, 1, _dataArray[0]);
                // Set the data
                for (int i = 1; i < CHUNK_SIZE; ++i) {
                    if (_dataArray[i] == data[index].data) {
                        ++(data[index].length);
                    } else {
                        data[++index].set(i, 1, _dataArray[i]);
                    }
                }
                // Set new state
                _state = VoxelStorageState::INTERVAL_TREE;
                // Create the tree
                _dataTree.initFromSortedArray(data, index + 1);

                dataLock.unlock();

                // Recycle memory
                _arrayRecycler->recycle(_dataArray);
                _dataArray = nullptr;

                totalContainerCompressions++;
            }

            IntervalTree<T> _dataTree; ///< Interval tree of voxel data

            T* _dataArray = nullptr; ///< pointer to an array of voxel data
            int _accessCount = 0; ///< Number of times the container was accessed this frame
            int _quietFrames = 0; ///< Number of frames since we have had heavy updates

            VoxelStorageState _state = VoxelStorageState::FLAT_ARRAY; ///< Current data structure state

            vcore::FixedSizeArrayRecycler<CHUNK_SIZE, T>* _arrayRecycler = nullptr; ///< For recycling the voxel arrays
        };

        /*template<typename T, size_t SIZE>
        inline SmartHandle<T, SIZE>::operator const T&() const {
            return m_container[m_index];
        }*/
        template<typename T, size_t SIZE>
        inline SmartHandle<T, SIZE>::operator const T&() const {
            return (m_container.getters[(size_t)m_container.getState()])(&m_container, m_index);
        }
        template<typename T, size_t SIZE>
        inline SmartHandle<T, SIZE>& SmartHandle<T, SIZE>::operator= (T data) {
            m_container.set(m_index, data);
            return *this;
        }
        template<typename T, size_t SIZE>
        inline SmartHandle<T, SIZE>& SmartHandle<T, SIZE>::operator= (const SmartHandle<T, SIZE>& o) {
            m_container.set(m_index, o.m_container[o.m_index]);
            return *this;
        }

        template<typename T, size_t SIZE>
        typename SmartVoxelContainer<T, SIZE>::Getter SmartVoxelContainer<T, SIZE>::getters[2] = {
            SmartVoxelContainer<T, SIZE>::getFlat,
            SmartVoxelContainer<T, SIZE>::getInterval
        };
        template<typename T, size_t SIZE>
        typename SmartVoxelContainer<T, SIZE>::Setter SmartVoxelContainer<T, SIZE>::setters[2] = {
            SmartVoxelContainer<T, SIZE>::setFlat,
            SmartVoxelContainer<T, SIZE>::setInterval
        };

    }
}
namespace vvox = vorb::voxel;

#endif // SmartVoxelContainer_h__