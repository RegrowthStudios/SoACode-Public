#pragma once
#include <string>
#include <stdint.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

// Note: char, size_t, And 3rd-Party Types May Be Used When Deemed Necessary

// Integer Values
typedef int8_t i8;
typedef int8_t sbyte;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t ui8;
typedef uint8_t ubyte;
typedef uint16_t ui16;
typedef uint32_t ui32;
typedef uint64_t ui64;

// Integer Vectors
typedef glm::detail::tvec2<i8> i8v2;
typedef glm::detail::tvec3<i8> i8v3;
typedef glm::detail::tvec4<i8> i8v4;
typedef glm::lowp_ivec2 i16v2;
typedef glm::lowp_ivec3 i16v3;
typedef glm::lowp_ivec4 i16v4;
typedef glm::mediump_ivec2 i32v2;
typedef glm::mediump_ivec3 i32v3;
typedef glm::mediump_ivec4 i32v4;
typedef glm::highp_ivec2 i64v2;
typedef glm::highp_ivec3 i64v3;
typedef glm::highp_ivec4 i64v4;
typedef glm::detail::tvec2<ui8> ui8v2;
typedef glm::detail::tvec3<ui8> ui8v3;
typedef glm::detail::tvec4<ui8> ui8v4;
typedef glm::lowp_uvec2 ui16v2;
typedef glm::lowp_uvec3 ui16v3;
typedef glm::lowp_uvec4 ui16v4;
typedef glm::mediump_uvec2 ui32v2;
typedef glm::mediump_uvec3 ui32v3;
typedef glm::mediump_uvec4 ui32v4;
typedef glm::highp_uvec2 ui64v2;
typedef glm::highp_uvec3 ui64v3;
typedef glm::highp_uvec4 ui64v4;

// Floating Point Values
typedef float f32;
typedef double f64;

// Floating Point Vectors
typedef glm::mediump_vec2 f32v2;
typedef glm::mediump_vec3 f32v3;
typedef glm::mediump_vec4 f32v4;
typedef glm::highp_vec2 f64v2;
typedef glm::highp_vec3 f64v3;
typedef glm::highp_vec4 f64v4;

// Floating Point Quaternions
typedef glm::mediump_quat f32q;
typedef glm::highp_quat f64q;

// Floating Point Matrices
typedef glm::mediump_mat2 f32m2;
typedef glm::mediump_mat3 f32m3;
typedef glm::mediump_mat4 f32m4;
typedef glm::highp_mat2 f64m2;
typedef glm::highp_mat3 f64m3;
typedef glm::highp_mat4 f64m4;

struct ColorRGBA8 {
    ColorRGBA8(ui8 r, ui8 g, ui8 b, ui8 a) :
        r(r), g(g), b(b), a(a) {}
    ColorRGBA8() : ColorRGBA8(0, 0, 0, 0) {}

    ui8 r;
    ui8 g;
    ui8 b;
    ui8 a;
};

struct ColorRGB8 {
    ColorRGB8(ui8 r, ui8 g, ui8 b) :
        r(r), g(g), b(b) {}
    ColorRGB8() {}

    ui8 r;
    ui8 g;
    ui8 b;
};

template<typename T> struct Array;

// A Better Array
struct ArrayBase {
public:
    ArrayBase(i32 elemSize) : _length(0), _elementSize(elemSize), _data(nullptr) {}
    ArrayBase(i32 elemSize, void* d, i32 l) : _length(l), _elementSize(elemSize) {
        if (_length > 0) {
            _data = new ui8[_elementSize * _length];
            memcpy(_data, d, _elementSize * _length);
        } else {
            _data = nullptr;
        }
    }
    ArrayBase(const ArrayBase& other) : ArrayBase(other._elementSize, other._data, other._length) {}
    ArrayBase& operator=(const ArrayBase& other) {
        _elementSize = other._elementSize;
        _length = other._length;
        if (other._data) {
            _data = new ui8[_elementSize * _length];
            memcpy(_data, other._data, _elementSize * _length);
        } else {
            _data = nullptr;
        }
        return *this;
    }
    ~ArrayBase() {
        if (_data) {
            delete[] _data;
            _data = nullptr;
            _length = 0;
        }
    }

    const i32& length() const {
        return _length;
    }

    const void setData(void* data, i32 len) {
        // Delete Previous Data
        if (_data) {
            delete[] _data;
            _data = nullptr;
            _length = 0;
        }

        // Set New Data
        if (data && len > 0) {
            _length = len;
            _data = new ui8[_length * _elementSize];
            memcpy(_data, data, _length * _elementSize);
        }
    }
    const void setData(i32 len = 0) {
        // Delete Previous Data
        if (_data) {
            delete[] _data;
            _data = nullptr;
            _length = 0;
        }

        // Set New Data
        if (len > 0) {
            _length = len;
            _data = new ui8[_length * _elementSize]();
        }
    }

    template<typename T>
    T& operator[] (size_t i) const {
        return ((T*)_data)[i];
    }
    template<typename T>
    T& at(size_t i) const {
        return ((T*)_data)[i];
    }
protected:
    void* _data;
    i32 _elementSize;
    i32 _length;
};

// A Better Array
template<typename T>
struct Array : public ArrayBase {
public:
    Array() : ArrayBase(sizeof(T)) {}

    T& operator[] (size_t i) const {
        return ((T*)_data)[i];
    }
    T& at(size_t i) const {
        return ((T*)_data)[i];
    }
};

// String
#define cString char*
#define cwString wchar_t*
#define nString std::string