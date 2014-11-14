///
/// SmartVoxelContainer.h
/// Vorb Engine
///
/// Created by Benjamin Arnold on 14 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// 
///

#pragma once

#ifndef SmartVoxelContainer_h__
#define SmartVoxelContainer_h__

#include "VoxelIntervalTree.h"
#include "Constants.h"

#include "FixedSizeArrayRecycler.hpp"

namespace vorb {
    namespace voxel {

        enum class VoxelStorageState { FLAT_ARRAY, INTERVAL_TREE };

        template <typename T>
        class SmartVoxelContainer {
        public:

            friend class Chunk;
            friend class ChunkGenerator;

            /// Constructor
            /// @param arrayRecycler: Pointer to a recycler. Template parameters must be
            /// <CHUNK_SIZE, T>
            SmartVoxelContainer(vcore::FixedSizeArrayRecycler<CHUNK_SIZE, T>* arrayRecycler) :
                _dataArray(nullptr), _accessCount(0), _quietFrames(0),
                _state(VoxelStorageState::FLAT_ARRAY), _arrayRecycler(arrayRecycler) {
            }

            /// Gets the element at index
            /// @param index: must be (0, CHUNK_SIZE]
            /// @return The element
            inline T get(ui16 index) const {
                // _accessCount++;
                if (_state == VoxelStorageState::INTERVAL_TREE) {
                    return _dataTree.getData(index);
                } else { //_state == FLAT_ARRAY
                    return _dataArray[index];
                }
            }

            /// Sets the element at index
            /// @param index: must be (0, CHUNK_SIZE]
            /// @param value: The value to set at index
            inline void set(ui16 index, T value) {
                _accessCount++;
                if (_state == VoxelStorageState::INTERVAL_TREE) {
                    _dataTree.insert(index, value);
                } else { //_state == FLAT_ARRAY
                    _dataArray[index] = value;
                }
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
                                            const std::vector <typename VoxelIntervalTree<T>::LightweightNode>& data) {
                _state = state;
                _accessCount = 0;
                _quietFrames = 0;
                if (_state == VoxelStorageState::INTERVAL_TREE) {
                    _dataTree.createFromSortedArray(data);
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

            #define QUIET_FRAMES_UNTIL_COMPRESS 60
            #define ACCESS_COUNT_UNTIL_DECOMPRESS 5

            /// Updates the container. Call once per frame
            inline void update() {
                // If access count is higher than the threshold, this is not a quiet frame
                if (_accessCount >= ACCESS_COUNT_UNTIL_DECOMPRESS) {
                    _quietFrames = 0;
                } else {
                    _quietFrames++;
                }

                if (_state == VoxelStorageState::INTERVAL_TREE) {
                    // Check if we should uncompress the data
                    if (_quietFrames == 0) {
                        _dataArray = _arrayRecycler->create();
                        uncompressIntoBuffer(_dataArray);
                        // Free memory
                        _dataTree.clear();
                        // Set the new state
                        _state = VoxelStorageState::FLAT_ARRAY;
                    }
                } else {
                    // Check if we should compress the data
                    if (_quietFrames >= QUIET_FRAMES_UNTIL_COMPRESS) {
                        // Sorted array for creating the interval tree
                        std::vector<VoxelIntervalTree<T>::LightweightNode> dataVector;
                        dataVector.reserve(CHUNK_WIDTH / 2);
                        dataVector.emplace_back(0, 1, _dataArray[0]);
                        // Set the data
                        for (int i = 1; i < CHUNK_SIZE; i++) {
                            if (_dataArray[i] == dataVector.back().data) {
                                dataVector.back().length++;
                            } else {
                                dataVector.emplace_back(i, 1, _dataArray[i]);
                            }
                        }
                        // Recycle memory
                        _arrayRecycler->recycle(_dataArray);
                        _dataArray = nullptr;
                        // Set new state
                        _state = VoxelStorageState::INTERVAL_TREE;
                        // Create the tree
                        _dataTree.createFromSortedArray(dataVector);
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
                } else {
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
            VoxelStorageState getState() { return _state; }
            T* getDataArray() { return _dataArray; }

        private: 
            VoxelIntervalTree<T> _dataTree; ///< Interval tree of voxel data
            T* _dataArray; ///< pointer to an array of voxel data
            int _accessCount; ///< Number of times the container was accessed this frame
            int _quietFrames; ///< Number of frames since we have had heavy updates

            VoxelStorageState _state; ///< Current data structure state

            vcore::FixedSizeArrayRecycler<CHUNK_SIZE, T>* _arrayRecycler; ///< For recycling the voxel arrays
        };
    }
}
// Namespace Alias
namespace vvoxel = vorb::voxel;

#endif // SmartVoxelContainer_h__