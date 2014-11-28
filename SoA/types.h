#pragma once

#ifndef types_h__
#define types_h__

#include <cstdint>
#include <memory>
#include <string>

#ifdef TYPES_GLM
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#endif

/************************************************************************/
/* Integer values                                                       */
/************************************************************************/
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

/************************************************************************/
/* Floating point values                                                */
/************************************************************************/
typedef float f32;
typedef double f64;

#ifdef TYPES_GLM

/************************************************************************/
/* GLM types                                                            */
/************************************************************************/
#ifdef glm_core_type_gentype2
// GLM Vec2 integer values
typedef glm::detail::tvec2<i8> i8v2;
typedef glm::lowp_ivec2 i16v2;
typedef glm::mediump_ivec2 i32v2;
typedef glm::highp_ivec2 i64v2;
typedef glm::detail::tvec2<ui8> ui8v2;
typedef glm::lowp_uvec2 ui16v2;
typedef glm::mediump_uvec2 ui32v2;
typedef glm::highp_uvec2 ui64v2;
// GLM Vec2 floating point values
typedef glm::mediump_vec2 f32v2;
typedef glm::highp_vec2 f64v2;
#endif
#ifdef glm_core_type_gentype3
// GLM Vec3 integer values
typedef glm::detail::tvec3<i8> i8v3;
typedef glm::lowp_ivec3 i16v3;
typedef glm::mediump_ivec3 i32v3;
typedef glm::highp_ivec3 i64v3;
typedef glm::detail::tvec3<ui8> ui8v3;
typedef glm::lowp_uvec3 ui16v3;
typedef glm::mediump_uvec3 ui32v3;
typedef glm::highp_uvec3 ui64v3;
// GLM Vec3 floating point values
typedef glm::mediump_vec3 f32v3;
typedef glm::highp_vec3 f64v3;
#endif
#ifdef glm_core_type_gentype4
// GLM Vec4 integer values
typedef glm::detail::tvec4<i8> i8v4;
typedef glm::lowp_ivec4 i16v4;
typedef glm::mediump_ivec4 i32v4;
typedef glm::highp_ivec4 i64v4;
typedef glm::detail::tvec4<ui8> ui8v4;
typedef glm::lowp_uvec4 ui16v4;
typedef glm::mediump_uvec4 ui32v4;
typedef glm::highp_uvec4 ui64v4;
// GLM Vec4 floating point values
typedef glm::mediump_vec4 f32v4;
typedef glm::highp_vec4 f64v4;
#endif

#ifdef GLM_GTC_quaternion
// GLM floating point quaternions
typedef glm::mediump_quat f32q;
typedef glm::highp_quat f64q;
#endif

// GLM floating point matrices
#ifdef glm_core_type_mat4x4
typedef glm::mediump_mat2 f32m2;
typedef glm::highp_mat2 f64m2;
#endif
#ifdef glm_core_type_mat3x3
typedef glm::mediump_mat3 f32m3;
typedef glm::highp_mat3 f64m3;
#endif
#ifdef glm_core_type_mat4x4
typedef glm::mediump_mat4 f32m4;
typedef glm::highp_mat4 f64m4;
#endif

#endif

/// RGBA color using 8-bit elements
struct ColorRGBA8 {
public:
    /// Create a color specifying all elements
    /// @param r: Red value
    /// @param g: Green value 
    /// @param b: Blue value
    /// @param a: Alpha value
    ColorRGBA8(ui8 r, ui8 g, ui8 b, ui8 a = 0xffu) :
        r(r), g(g), b(b), a(a) {
        // Empty
    }
    /// Create a color specifying all elements in integer values
    /// @param r: Red value
    /// @param g: Green value
    /// @param b: Blue value
    /// @param a: Alpha value
    ColorRGBA8(i32 r, i32 g, i32 b, i32 a = 1.0f) : ColorRGBA8((ui8)r, (ui8)g, (ui8)b, (ui8)a) {
        // Empty
    }
    /// Create a color specifying all elements in floating point values
    /// @param r: Red value [0.0f, 1.0f]
    /// @param g: Green value [0.0f, 1.0f]
    /// @param b: Blue value [0.0f, 1.0f]
    /// @param a: Alpha value [0.0f, 1.0f]
    ColorRGBA8(f32 r, f32 g, f32 b, f32 a = 1.0f) : ColorRGBA8(
        ((ui8)r * 255.0f),
        ((ui8)g * 255.0f),
        ((ui8)b * 255.0f),
        ((ui8)a * 255.0f)) {
        // Empty
    }
    /// Default black color
    ColorRGBA8() : ColorRGBA8(0, 0, 0) {
        // Empty
    }

    /// Access a color element by its index
    /// @param i: Color index in range [0,3]
    /// @return Reference to color element
    const ui8& operator[] (size_t i) const {
        return data[i];
    }
    /// Access a color element by its index
    /// @param i: Color index in range [0,3]
    /// @return Reference to color element
    ui8& operator[] (size_t i) {
        return data[i];
    }

    union {
        struct {
            ui8 r; ///< Red value
            ui8 g; ///< Green value
            ui8 b; ///< Blue value
            ui8 a; ///< Alpha value
        };
        ui8 data[4]; ///< RGBA values stored in array
    };
};
typedef ColorRGBA8 color4; ///< Simple name for ColorRGBA8

/// RGB color using 8-bit elements
struct ColorRGB8 {
public:
    /// Create a color specifying all elements
    /// @param r: Red value
    /// @param g: Green value 
    /// @param b: Blue value
    ColorRGB8(ui8 r, ui8 g, ui8 b) :
        r(r), g(g), b(b) {
        // Empty
    }
    /// Create a color specifying all elements in integer values
    /// @param r: Red value
    /// @param g: Green value
    /// @param b: Blue value
    ColorRGB8(i32 r, i32 g, i32 b) : ColorRGB8((ui8)r, (ui8)g, (ui8)b) {
        // Empty
    }
    /// Create a color specifying all elements in floating point values
    /// @param r: Red value [0.0f, 1.0f]
    /// @param g: Green value [0.0f, 1.0f]
    /// @param b: Blue value [0.0f, 1.0f]
    ColorRGB8(f32 r, f32 g, f32 b) : ColorRGB8(
        ((ui8)r * 255.0f),
        ((ui8)g * 255.0f),
        ((ui8)b * 255.0f)) {
        // Empty
    }
    /// Default black color
    ColorRGB8() : ColorRGB8(0, 0, 0) {
        // Empty
    }

    /// Access a color element by its index
    /// @param i: Color index in range [0,2]
    /// @return Reference to color element
    const ui8& operator[] (size_t i) const {
        return data[i];
    }
    /// Access a color element by its index
    /// @param i: Color index in range [0,2]
    /// @return Reference to color element
    ui8& operator[] (size_t i) {
        return data[i];
    }

    union {
        struct {
            ui8 r; ///< Red value
            ui8 g; ///< Green value
            ui8 b; ///< Blue value
        };
        ui8 data[3]; ///< RGB values stored in array
    };
};
typedef ColorRGB8 color3; ///< Simple name for ColorRGB8

/// A common array type for unknown values
class ArrayBase {
    public:
        ArrayBase(ui32 elemSize) :
            _elementSize(elemSize) {
            // Empty
        }
        ArrayBase(ui32 elemSize, void* d, size_t l) : ArrayBase(elemSize) {
            setData(d, l);
        }

        const size_t& getLength() const {
            return _length;
        }

        void setData(void* data, size_t len) {
            _length = len;
            if (_length > 0) {
                // Create a new internal array
                _dataShared.reset(new ui8[_elementSize * _length], [] (ui8* p) { delete[] p; });
                _data = _dataShared.get();

                // Copy data
                if (data) memcpy(_data, data, _elementSize * _length);
            } else {
                // Delete any old data
                _dataShared.reset();
                _data = nullptr;
            }
        }
        void setData(size_t len = 0) {
            setData(nullptr, len);
        }

        template<typename T>
        T& operator[] (size_t i)  {
            return ((T*)_data)[i];
        }
        template<typename T>
        const T& operator[] (size_t i) const {
            return ((T*)_data)[i];
        }

        template<typename T>
        T& at(size_t i){
            return ((T*)_data)[i];
        }
        template<typename T>
        const T& at(size_t i) const {
            return ((T*)_data)[i];
        }

    protected:
        std::shared_ptr<ui8> _dataShared;
        void* _data = nullptr;
        ui32 _elementSize = 0;
        size_t _length = 0;
};

/// An array of known value types
template<typename T>
class Array : public ArrayBase {
    public:
        Array() : ArrayBase(sizeof(T)) {
            // Empty
        }
        Array(T* d, size_t l) : ArrayBase(sizeof(T), d, l) {
            // Empty
        }

        T& operator[] (size_t i) {
            return ((T*)_data)[i];
        }
        const T& operator[] (size_t i) const {
            return ((T*)_data)[i];
        }
        T& at(size_t i) {
            return ((T*)_data)[i];
        }
        const T& at(size_t i) const {
            return ((T*)_data)[i];
        }
};

// String
#define cString char*
#define cwString wchar_t*
#define nString std::string

/// Pointer offset
#define offsetptr(s, m) ((void*)offsetof(s, m))

#endif // types_h__
