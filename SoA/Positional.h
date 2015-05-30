//
// Positional.h
// Seed of Andromeda
//
// Created by Cristian Zaloj on 28 May 2015
// Copyright 2014 Regrowth Studios
// All Rights Reserved
//
// Summary:
// 
//

#pragma once

#ifndef Positional_h__
#define Positional_h__

// TODO(Cristian): Some constants don't belong here
#include "Constants.h"

// TODO(Cristian): Move this to Vorb types.h
#define soffsetof(T, M) static_cast<size_t>((ptrdiff_t)&(((T*)nullptr)->M))

typedef i32 VoxelPositionType;
typedef glm::detail::tvec3<VoxelPositionType> VoxelVectorType;

inline VoxelPositionType wrapInBounds(VoxelPositionType v) {
    return v & (CHUNK_WIDTH - 1);
}

class VoxelIterablePositionRawX;
class VoxelIterablePositionRawY;
class VoxelIterablePositionRawZ;
template<size_t BITS> class VoxelIterablePositionWrapX;
template<size_t BITS> class VoxelIterablePositionWrapY;
template<size_t BITS> class VoxelIterablePositionWrapZ;
template<VoxelPositionType MIN, VoxelPositionType MAX> class VoxelIterablePositionClampX;
template<VoxelPositionType MIN, VoxelPositionType MAX> class VoxelIterablePositionClampY;
template<VoxelPositionType MIN, VoxelPositionType MAX> class VoxelIterablePositionClampZ;

struct VoxelIterablePosition {
public:
    VoxelPositionType x, y, z;

    VoxelIterablePositionRawX& rx() {
        return reinterpret_cast<VoxelIterablePositionRawX&>(*this);
    }
    VoxelIterablePositionRawY& ry() {
        return reinterpret_cast<VoxelIterablePositionRawY&>(*this);
    }
    VoxelIterablePositionRawZ& rz() {
        return reinterpret_cast<VoxelIterablePositionRawZ&>(*this);
    }

    template<size_t BITS = 5>
    VoxelIterablePositionWrapX<BITS>& wx() {
        return reinterpret_cast<VoxelIterablePositionWrapX<BITS>&>(*this);
    }
    template<size_t BITS = 5>
    VoxelIterablePositionWrapY<BITS>& wy() {
        return reinterpret_cast<VoxelIterablePositionWrapY<BITS>&>(*this);
    }
    template<size_t BITS = 5>
    VoxelIterablePositionWrapZ<BITS>& wz() {
        return reinterpret_cast<VoxelIterablePositionWrapZ<BITS>&>(*this);
    }
    
    template<size_t MIN = 0, size_t MAX = CHUNK_WIDTH - 1>
    VoxelIterablePositionClampX<MIN, MAX>& cx() {
        return reinterpret_cast<VoxelIterablePositionClampX<MIN, MAX>&>(*this);
    }
    template<size_t MIN = 0, size_t MAX = CHUNK_WIDTH - 1>
    VoxelIterablePositionClampY<MIN, MAX>& cy() {
        return reinterpret_cast<VoxelIterablePositionClampY<MIN, MAX>&>(*this);
    }
    template<size_t MIN = 0, size_t MAX = CHUNK_WIDTH - 1>
    VoxelIterablePositionClampZ<MIN, MAX>& cz() {
        return reinterpret_cast<VoxelIterablePositionClampZ<MIN, MAX>&>(*this);
    }

    operator VoxelVectorType() const {
    }
};

// Check alignment against glm types
static_assert(soffsetof(VoxelIterablePosition, x) == soffsetof(VoxelVectorType, x), "VoxelIterablePosition X is misaligned");
static_assert(soffsetof(VoxelIterablePosition, y) == soffsetof(VoxelVectorType, y), "VoxelIterablePosition Y is misaligned");
static_assert(soffsetof(VoxelIterablePosition, z) == soffsetof(VoxelVectorType, z), "VoxelIterablePosition Z is misaligned");
static_assert(sizeof(VoxelIterablePosition) == sizeof(VoxelVectorType), "VoxelIterablePosition is of wrong size");

/*! @brief This operator class modifies value without any modifications.
 * 
 * This class only exists to allow for templated code to take advantage of different iterator forms.
 * <br/>
 * pos.x += 5 performs the same modifications as pos.rx() += 5;
 * <br/>
 * although (pos.rx() += 5).ry() += 5 is a valid statement
 */
template<VoxelPositionType VoxelIterablePosition::*M>
struct VoxelIterablePositionRaw : public VoxelIterablePosition {
public:
    VoxelIterablePositionRaw& operator+= (VoxelPositionType v) {
        this->*M += v;
        return *this;
    }
    VoxelIterablePositionRaw operator+ (VoxelPositionType v) {
        VoxelIterablePositionRaw pos(*this);
        pos += v;
        return pos;
    }
    operator VoxelIterablePosition() const {
        return *this;
    }
};
class VoxelIterablePositionRawX : public VoxelIterablePositionRaw<&VoxelIterablePosition::x> {};
class VoxelIterablePositionRawY : public VoxelIterablePositionRaw<&VoxelIterablePosition::y> {};
class VoxelIterablePositionRawZ : public VoxelIterablePositionRaw<&VoxelIterablePosition::z> {};

/*! @brief This operator class wraps values at the end of a modification between [0, (1 << BITS) - 1]
 */
template<size_t BITS, VoxelPositionType VoxelIterablePosition::*M>
struct VoxelIterablePositionWrap : public VoxelIterablePosition {
public:
    static const VoxelPositionType BIT_MASK = (1 << BITS) - 1;

    void wrap() {
        this->*M &= BIT_MASK;
    }
    VoxelIterablePositionWrap& operator+= (VoxelPositionType v) {
        this->*M += v;
        this->*M &= BIT_MASK;
        return *this;
    }
    VoxelIterablePositionWrap operator+ (VoxelPositionType v) {
        VoxelIterablePositionWrap pos(*this);
        pos += v;
        return pos;
    }
    operator VoxelIterablePosition() const {
        return *this;
    }
};
template<size_t BITS>
class VoxelIterablePositionWrapX : public VoxelIterablePositionWrap<BITS, &VoxelIterablePosition::x> {};
template<size_t BITS>
class VoxelIterablePositionWrapY : public VoxelIterablePositionWrap<BITS, &VoxelIterablePosition::y> {};
template<size_t BITS>
class VoxelIterablePositionWrapZ : public VoxelIterablePositionWrap<BITS, &VoxelIterablePosition::z> {};

/*! @brief This operator class clamps values at the end of a modification between [MIN, MAX]
 */
template<VoxelPositionType MIN, VoxelPositionType MAX, VoxelPositionType VoxelIterablePosition::*M>
struct VoxelIterablePositionClamp : public VoxelIterablePosition {
public:
    void clamp() {
        if (this->*M > MAX) this->*M = MAX;
        else if (this->*M < MIN) this->*M = MIN;
    }
    VoxelIterablePositionClamp& operator+= (VoxelPositionType v) {
        this->*M += v;
        if (this->*M > MAX) this->*M = MAX;
        else if (this->*M < MIN) this->*M = MIN;
        return *this;
    }
    VoxelIterablePositionClamp operator+ (VoxelPositionType v) {
        VoxelIterablePositionClamp pos(*this);
        pos += v;
        return pos;
    }
    operator VoxelIterablePosition() const {
        return *this;
    }
};
template<VoxelPositionType MIN, VoxelPositionType MAX>
class VoxelIterablePositionClampX : public VoxelIterablePositionClamp<MIN, MAX, &VoxelIterablePosition::x> {};
template<VoxelPositionType MIN, VoxelPositionType MAX>
class VoxelIterablePositionClampY : public VoxelIterablePositionClamp<MIN, MAX, &VoxelIterablePosition::y> {};
template<VoxelPositionType MIN, VoxelPositionType MAX>
class VoxelIterablePositionClampZ : public VoxelIterablePositionClamp<MIN, MAX, &VoxelIterablePosition::z> {};

struct VoxelPosition {
public:
    VoxelIterablePosition chunk;
    VoxelIterablePosition voxel;
};

#endif // Positional_h__
