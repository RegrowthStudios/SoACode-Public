#pragma once
#include <string>
#include <cstdint>

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
public:

    ColorRGBA8(ui8 r, ui8 g, ui8 b, ui8 a)
    : r(r), g(g), b(b), a(a) {
        // empty
    }

    ColorRGBA8()
    : r(0), g(0), b(0), a(0) {
        // empty
    }

    ui8 r;
    ui8 g;
    ui8 b;
    ui8 a;
};

namespace color {
    extern ColorRGBA8 Transparent;
    extern ColorRGBA8 AliceBlue;
    extern ColorRGBA8 AntiqueWhite;
    extern ColorRGBA8 Aqua;
    extern ColorRGBA8 Aquamarine;
    extern ColorRGBA8 Azure;
    extern ColorRGBA8 Beige;
    extern ColorRGBA8 Bisque;
    extern ColorRGBA8 Black;
    extern ColorRGBA8 BlanchedAlmond;
    extern ColorRGBA8 Blue;
    extern ColorRGBA8 BlueViolet;
    extern ColorRGBA8 Brown;
    extern ColorRGBA8 BurlyWood;
    extern ColorRGBA8 CadetBlue;
    extern ColorRGBA8 Chartreuse;
    extern ColorRGBA8 Chocolate;
    extern ColorRGBA8 Coral;
    extern ColorRGBA8 CornflowerBlue;
    extern ColorRGBA8 Cornsilk;
    extern ColorRGBA8 Crimson;
    extern ColorRGBA8 Cyan;
    extern ColorRGBA8 DarkBlue;
    extern ColorRGBA8 DarkCyan;
    extern ColorRGBA8 DarkGoldenrod;
    extern ColorRGBA8 DarkGray;
    extern ColorRGBA8 DarkGreen;
    extern ColorRGBA8 DarkKhaki;
    extern ColorRGBA8 DarkMagenta;
    extern ColorRGBA8 DarkOliveGreen;
    extern ColorRGBA8 DarkOrange;
    extern ColorRGBA8 DarkOrchid;
    extern ColorRGBA8 DarkRed;
    extern ColorRGBA8 DarkSalmon;
    extern ColorRGBA8 DarkSeaGreen;
    extern ColorRGBA8 DarkSlateBlue;
    extern ColorRGBA8 DarkSlateGray;
    extern ColorRGBA8 DarkTurquoise;
    extern ColorRGBA8 DarkViolet;
    extern ColorRGBA8 DeepPink;
    extern ColorRGBA8 DeepSkyBlue;
    extern ColorRGBA8 DimGray;
    extern ColorRGBA8 DodgerBlue;
    extern ColorRGBA8 Firebrick;
    extern ColorRGBA8 FloralWhite;
    extern ColorRGBA8 ForestGreen;
    extern ColorRGBA8 Fuchsia;
    extern ColorRGBA8 Gainsboro;
    extern ColorRGBA8 GhostWhite;
    extern ColorRGBA8 Gold;
    extern ColorRGBA8 Goldenrod;
    extern ColorRGBA8 Gray;
    extern ColorRGBA8 Green;
    extern ColorRGBA8 GreenYellow;
    extern ColorRGBA8 Honeydew;
    extern ColorRGBA8 HotPink;
    extern ColorRGBA8 IndianRed;
    extern ColorRGBA8 Indigo;
    extern ColorRGBA8 Ivory;
    extern ColorRGBA8 Khaki;
    extern ColorRGBA8 Lavender;
    extern ColorRGBA8 LavenderBlush;
    extern ColorRGBA8 LawnGreen;
    extern ColorRGBA8 LemonChiffon;
    extern ColorRGBA8 LightBlue;
    extern ColorRGBA8 LightCoral;
    extern ColorRGBA8 LightCyan;
    extern ColorRGBA8 LightGoldenrodYellow;
    extern ColorRGBA8 LightGreen;
    extern ColorRGBA8 LightGray;
    extern ColorRGBA8 LightPink;
    extern ColorRGBA8 LightSalmon;
    extern ColorRGBA8 LightSeaGreen;
    extern ColorRGBA8 LightSkyBlue;
    extern ColorRGBA8 LightSlateGray;
    extern ColorRGBA8 LightSteelBlue;
    extern ColorRGBA8 LightYellow;
    extern ColorRGBA8 Lime;
    extern ColorRGBA8 LimeGreen;
    extern ColorRGBA8 Linen;
    extern ColorRGBA8 Magenta;
    extern ColorRGBA8 Maroon;
    extern ColorRGBA8 MediumAquamarine;
    extern ColorRGBA8 MediumBlue;
    extern ColorRGBA8 MediumOrchid;
    extern ColorRGBA8 MediumPurple;
    extern ColorRGBA8 MediumSeaGreen;
    extern ColorRGBA8 MediumSlateBlue;
    extern ColorRGBA8 MediumSpringGreen;
    extern ColorRGBA8 MediumTurquoise;
    extern ColorRGBA8 MediumVioletRed;
    extern ColorRGBA8 MidnightBlue;
    extern ColorRGBA8 MintCream;
    extern ColorRGBA8 MistyRose;
    extern ColorRGBA8 Moccasin;
    extern ColorRGBA8 NavajoWhite;
    extern ColorRGBA8 Navy;
    extern ColorRGBA8 OldLace;
    extern ColorRGBA8 Olive;
    extern ColorRGBA8 OliveDrab;
    extern ColorRGBA8 Orange;
    extern ColorRGBA8 OrangeRed;
    extern ColorRGBA8 Orchid;
    extern ColorRGBA8 PaleGoldenrod;
    extern ColorRGBA8 PaleGreen;
    extern ColorRGBA8 PaleTurquoise;
    extern ColorRGBA8 PaleVioletRed;
    extern ColorRGBA8 PapayaWhip;
    extern ColorRGBA8 PeachPuff;
    extern ColorRGBA8 Peru;
    extern ColorRGBA8 Pink;
    extern ColorRGBA8 Plum;
    extern ColorRGBA8 PowderBlue;
    extern ColorRGBA8 Purple;
    extern ColorRGBA8 Red;
    extern ColorRGBA8 RosyBrown;
    extern ColorRGBA8 RoyalBlue;
    extern ColorRGBA8 SaddleBrown;
    extern ColorRGBA8 Salmon;
    extern ColorRGBA8 SandyBrown;
    extern ColorRGBA8 SeaGreen;
    extern ColorRGBA8 SeaShell;
    extern ColorRGBA8 Sienna;
    extern ColorRGBA8 Silver;
    extern ColorRGBA8 SkyBlue;
    extern ColorRGBA8 SlateBlue;
    extern ColorRGBA8 SlateGray;
    extern ColorRGBA8 Snow;
    extern ColorRGBA8 SpringGreen;
    extern ColorRGBA8 SteelBlue;
    extern ColorRGBA8 Tan;
    extern ColorRGBA8 Teal;
    extern ColorRGBA8 Thistle;
    extern ColorRGBA8 Tomato;
    extern ColorRGBA8 Turquoise;
    extern ColorRGBA8 Violet;
    extern ColorRGBA8 Wheat;
    extern ColorRGBA8 White;
    extern ColorRGBA8 WhiteSmoke;
    extern ColorRGBA8 Yellow;
    extern ColorRGBA8 YellowGreen;
}


struct ColorRGB8 {
public:
    ColorRGB8(ui8 r, ui8 g, ui8 b)
    : r(r), g(g), b(b) {
        // empty
    }

    ColorRGB8()
    : r(0), g(0), b(0) {
        // empty
    }

    ui8 r;
    ui8 g;
    ui8 b;
};


template<typename T> struct Array;

// A Better Array
struct ArrayBase {
    public:
        ArrayBase(i32 elemSize)
        : _data(nullptr), _elementSize(elemSize), _length(0) {
            // empty
        }

        ArrayBase(i32 elemSize, void* d, i32 l)
        : _elementSize(elemSize), _length(l) {
            if (_length > 0) {
                _data = new ui8[_elementSize * _length];
                memcpy(_data, d, _elementSize * _length);
            } else {
                _data = nullptr;
            }
        }

        ArrayBase(const ArrayBase& other)
        : ArrayBase(other._elementSize, other._data, other._length) {
            // empty
        }

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
                delete[] static_cast<ui8*>(_data);
                _data = nullptr;
                _length = 0;
            }
        }

        const i32& length() const {
            return _length;
        }

        void setData(void* data, i32 len) {
            // Delete Previous Data
            if (_data) {
                delete[] static_cast<ui8*>(_data);
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

        void setData(i32 len = 0) {
            // Delete Previous Data
            if (_data) {
                delete[] static_cast<ui8*>(_data);
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
        Array() : ArrayBase(sizeof(T)) {
            // empty
        }

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
