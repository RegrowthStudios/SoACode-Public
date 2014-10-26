#include "stdafx.h"
#include "colors.h"

#include <cmath>

// All Color Names And Values Are From The C# XNA Framework
const ColorRGBA8 color::Transparent = ColorRGBA8(0, 0, 0, 0);
const ColorRGBA8 color::AliceBlue = ColorRGBA8(240, 248, 255, 255);
const ColorRGBA8 color::AntiqueWhite = ColorRGBA8(250, 235, 215, 255);
const ColorRGBA8 color::Aqua = ColorRGBA8(0, 255, 255, 255);
const ColorRGBA8 color::Aquamarine = ColorRGBA8(127, 255, 212, 255);
const ColorRGBA8 color::Azure = ColorRGBA8(240, 255, 255, 255);
const ColorRGBA8 color::Beige = ColorRGBA8(245, 245, 220, 255);
const ColorRGBA8 color::Bisque = ColorRGBA8(255, 228, 196, 255);
const ColorRGBA8 color::Black = ColorRGBA8(0, 0, 0, 255);
const ColorRGBA8 color::BlanchedAlmond = ColorRGBA8(255, 235, 205, 255);
const ColorRGBA8 color::Blue = ColorRGBA8(0, 0, 255, 255);
const ColorRGBA8 color::BlueViolet = ColorRGBA8(138, 43, 226, 255);
const ColorRGBA8 color::Brown = ColorRGBA8(165, 42, 42, 255);
const ColorRGBA8 color::BurlyWood = ColorRGBA8(222, 184, 135, 255);
const ColorRGBA8 color::CadetBlue = ColorRGBA8(95, 158, 160, 255);
const ColorRGBA8 color::Chartreuse = ColorRGBA8(127, 255, 0, 255);
const ColorRGBA8 color::Chocolate = ColorRGBA8(210, 105, 30, 255);
const ColorRGBA8 color::Coral = ColorRGBA8(255, 127, 80, 255);
const ColorRGBA8 color::CornflowerBlue = ColorRGBA8(100, 149, 237, 255);
const ColorRGBA8 color::Cornsilk = ColorRGBA8(255, 248, 220, 255);
const ColorRGBA8 color::Crimson = ColorRGBA8(220, 20, 60, 255);
const ColorRGBA8 color::Cyan = ColorRGBA8(0, 255, 255, 255);
const ColorRGBA8 color::DarkBlue = ColorRGBA8(0, 0, 139, 255);
const ColorRGBA8 color::DarkCyan = ColorRGBA8(0, 139, 139, 255);
const ColorRGBA8 color::DarkGoldenrod = ColorRGBA8(184, 134, 11, 255);
const ColorRGBA8 color::DarkGray = ColorRGBA8(169, 169, 169, 255);
const ColorRGBA8 color::DarkGreen = ColorRGBA8(0, 100, 0, 255);
const ColorRGBA8 color::DarkKhaki = ColorRGBA8(189, 183, 107, 255);
const ColorRGBA8 color::DarkMagenta = ColorRGBA8(139, 0, 139, 255);
const ColorRGBA8 color::DarkOliveGreen = ColorRGBA8(85, 107, 47, 255);
const ColorRGBA8 color::DarkOrange = ColorRGBA8(255, 140, 0, 255);
const ColorRGBA8 color::DarkOrchid = ColorRGBA8(153, 50, 204, 255);
const ColorRGBA8 color::DarkRed = ColorRGBA8(139, 0, 0, 255);
const ColorRGBA8 color::DarkSalmon = ColorRGBA8(233, 150, 122, 255);
const ColorRGBA8 color::DarkSeaGreen = ColorRGBA8(143, 188, 139, 255);
const ColorRGBA8 color::DarkSlateBlue = ColorRGBA8(72, 61, 139, 255);
const ColorRGBA8 color::DarkSlateGray = ColorRGBA8(47, 79, 79, 255);
const ColorRGBA8 color::DarkTurquoise = ColorRGBA8(0, 206, 209, 255);
const ColorRGBA8 color::DarkViolet = ColorRGBA8(148, 0, 211, 255);
const ColorRGBA8 color::DeepPink = ColorRGBA8(255, 20, 147, 255);
const ColorRGBA8 color::DeepSkyBlue = ColorRGBA8(0, 191, 255, 255);
const ColorRGBA8 color::DimGray = ColorRGBA8(105, 105, 105, 255);
const ColorRGBA8 color::DodgerBlue = ColorRGBA8(30, 144, 255, 255);
const ColorRGBA8 color::Firebrick = ColorRGBA8(178, 34, 34, 255);
const ColorRGBA8 color::FloralWhite = ColorRGBA8(255, 250, 240, 255);
const ColorRGBA8 color::ForestGreen = ColorRGBA8(34, 139, 34, 255);
const ColorRGBA8 color::Fuchsia = ColorRGBA8(255, 0, 255, 255);
const ColorRGBA8 color::Gainsboro = ColorRGBA8(220, 220, 220, 255);
const ColorRGBA8 color::GhostWhite = ColorRGBA8(248, 248, 255, 255);
const ColorRGBA8 color::Gold = ColorRGBA8(255, 215, 0, 255);
const ColorRGBA8 color::Goldenrod = ColorRGBA8(218, 165, 32, 255);
const ColorRGBA8 color::Gray = ColorRGBA8(128, 128, 128, 255);
const ColorRGBA8 color::Green = ColorRGBA8(0, 128, 0, 255);
const ColorRGBA8 color::GreenYellow = ColorRGBA8(173, 255, 47, 255);
const ColorRGBA8 color::Honeydew = ColorRGBA8(240, 255, 240, 255);
const ColorRGBA8 color::HotPink = ColorRGBA8(255, 105, 180, 255);
const ColorRGBA8 color::IndianRed = ColorRGBA8(205, 92, 92, 255);
const ColorRGBA8 color::Indigo = ColorRGBA8(75, 0, 130, 255);
const ColorRGBA8 color::Ivory = ColorRGBA8(255, 255, 240, 255);
const ColorRGBA8 color::Khaki = ColorRGBA8(240, 230, 140, 255);
const ColorRGBA8 color::Lavender = ColorRGBA8(230, 230, 250, 255);
const ColorRGBA8 color::LavenderBlush = ColorRGBA8(255, 240, 245, 255);
const ColorRGBA8 color::LawnGreen = ColorRGBA8(124, 252, 0, 255);
const ColorRGBA8 color::LemonChiffon = ColorRGBA8(255, 250, 205, 255);
const ColorRGBA8 color::LightBlue = ColorRGBA8(173, 216, 230, 255);
const ColorRGBA8 color::LightCoral = ColorRGBA8(240, 128, 128, 255);
const ColorRGBA8 color::LightCyan = ColorRGBA8(224, 255, 255, 255);
const ColorRGBA8 color::LightGoldenrodYellow = ColorRGBA8(250, 250, 210, 255);
const ColorRGBA8 color::LightGreen = ColorRGBA8(144, 238, 144, 255);
const ColorRGBA8 color::LightGray = ColorRGBA8(211, 211, 211, 255);
const ColorRGBA8 color::LightPink = ColorRGBA8(255, 182, 193, 255);
const ColorRGBA8 color::LightSalmon = ColorRGBA8(255, 160, 122, 255);
const ColorRGBA8 color::LightSeaGreen = ColorRGBA8(32, 178, 170, 255);
const ColorRGBA8 color::LightSkyBlue = ColorRGBA8(135, 206, 250, 255);
const ColorRGBA8 color::LightSlateGray = ColorRGBA8(119, 136, 153, 255);
const ColorRGBA8 color::LightSteelBlue = ColorRGBA8(176, 196, 222, 255);
const ColorRGBA8 color::LightYellow = ColorRGBA8(255, 255, 224, 255);
const ColorRGBA8 color::Lime = ColorRGBA8(0, 255, 0, 255);
const ColorRGBA8 color::LimeGreen = ColorRGBA8(50, 205, 50, 255);
const ColorRGBA8 color::Linen = ColorRGBA8(250, 240, 230, 255);
const ColorRGBA8 color::Magenta = ColorRGBA8(255, 0, 255, 255);
const ColorRGBA8 color::Maroon = ColorRGBA8(128, 0, 0, 255);
const ColorRGBA8 color::MediumAquamarine = ColorRGBA8(102, 205, 170, 255);
const ColorRGBA8 color::MediumBlue = ColorRGBA8(0, 0, 205, 255);
const ColorRGBA8 color::MediumOrchid = ColorRGBA8(186, 85, 211, 255);
const ColorRGBA8 color::MediumPurple = ColorRGBA8(147, 112, 219, 255);
const ColorRGBA8 color::MediumSeaGreen = ColorRGBA8(60, 179, 113, 255);
const ColorRGBA8 color::MediumSlateBlue = ColorRGBA8(123, 104, 238, 255);
const ColorRGBA8 color::MediumSpringGreen = ColorRGBA8(0, 250, 154, 255);
const ColorRGBA8 color::MediumTurquoise = ColorRGBA8(72, 209, 204, 255);
const ColorRGBA8 color::MediumVioletRed = ColorRGBA8(199, 21, 133, 255);
const ColorRGBA8 color::MidnightBlue = ColorRGBA8(25, 25, 112, 255);
const ColorRGBA8 color::MintCream = ColorRGBA8(245, 255, 250, 255);
const ColorRGBA8 color::MistyRose = ColorRGBA8(255, 228, 225, 255);
const ColorRGBA8 color::Moccasin = ColorRGBA8(255, 228, 181, 255);
const ColorRGBA8 color::NavajoWhite = ColorRGBA8(255, 222, 173, 255);
const ColorRGBA8 color::Navy = ColorRGBA8(0, 0, 128, 255);
const ColorRGBA8 color::OldLace = ColorRGBA8(253, 245, 230, 255);
const ColorRGBA8 color::Olive = ColorRGBA8(128, 128, 0, 255);
const ColorRGBA8 color::OliveDrab = ColorRGBA8(107, 142, 35, 255);
const ColorRGBA8 color::Orange = ColorRGBA8(255, 165, 0, 255);
const ColorRGBA8 color::OrangeRed = ColorRGBA8(255, 69, 0, 255);
const ColorRGBA8 color::Orchid = ColorRGBA8(218, 112, 214, 255);
const ColorRGBA8 color::PaleGoldenrod = ColorRGBA8(238, 232, 170, 255);
const ColorRGBA8 color::PaleGreen = ColorRGBA8(152, 251, 152, 255);
const ColorRGBA8 color::PaleTurquoise = ColorRGBA8(175, 238, 238, 255);
const ColorRGBA8 color::PaleVioletRed = ColorRGBA8(219, 112, 147, 255);
const ColorRGBA8 color::PapayaWhip = ColorRGBA8(255, 239, 213, 255);
const ColorRGBA8 color::PeachPuff = ColorRGBA8(255, 218, 185, 255);
const ColorRGBA8 color::Peru = ColorRGBA8(205, 133, 63, 255);
const ColorRGBA8 color::Pink = ColorRGBA8(255, 192, 203, 255);
const ColorRGBA8 color::Plum = ColorRGBA8(221, 160, 221, 255);
const ColorRGBA8 color::PowderBlue = ColorRGBA8(176, 224, 230, 255);
const ColorRGBA8 color::Purple = ColorRGBA8(128, 0, 128, 255);
const ColorRGBA8 color::Red = ColorRGBA8(255, 0, 0, 255);
const ColorRGBA8 color::RosyBrown = ColorRGBA8(188, 143, 143, 255);
const ColorRGBA8 color::RoyalBlue = ColorRGBA8(65, 105, 225, 255);
const ColorRGBA8 color::SaddleBrown = ColorRGBA8(139, 69, 19, 255);
const ColorRGBA8 color::Salmon = ColorRGBA8(250, 128, 114, 255);
const ColorRGBA8 color::SandyBrown = ColorRGBA8(244, 164, 96, 255);
const ColorRGBA8 color::SeaGreen = ColorRGBA8(46, 139, 87, 255);
const ColorRGBA8 color::SeaShell = ColorRGBA8(255, 245, 238, 255);
const ColorRGBA8 color::Sienna = ColorRGBA8(160, 82, 45, 255);
const ColorRGBA8 color::Silver = ColorRGBA8(192, 192, 192, 255);
const ColorRGBA8 color::SkyBlue = ColorRGBA8(135, 206, 235, 255);
const ColorRGBA8 color::SlateBlue = ColorRGBA8(106, 90, 205, 255);
const ColorRGBA8 color::SlateGray = ColorRGBA8(112, 128, 144, 255);
const ColorRGBA8 color::Snow = ColorRGBA8(255, 250, 250, 255);
const ColorRGBA8 color::SpringGreen = ColorRGBA8(0, 255, 127, 255);
const ColorRGBA8 color::SteelBlue = ColorRGBA8(70, 130, 180, 255);
const ColorRGBA8 color::Tan = ColorRGBA8(210, 180, 140, 255);
const ColorRGBA8 color::Teal = ColorRGBA8(0, 128, 128, 255);
const ColorRGBA8 color::Thistle = ColorRGBA8(216, 191, 216, 255);
const ColorRGBA8 color::Tomato = ColorRGBA8(255, 99, 71, 255);
const ColorRGBA8 color::Turquoise = ColorRGBA8(64, 224, 208, 255);
const ColorRGBA8 color::Violet = ColorRGBA8(238, 130, 238, 255);
const ColorRGBA8 color::Wheat = ColorRGBA8(245, 222, 179, 255);
const ColorRGBA8 color::White = ColorRGBA8(255, 255, 255, 255);
const ColorRGBA8 color::WhiteSmoke = ColorRGBA8(245, 245, 245, 255);
const ColorRGBA8 color::Yellow = ColorRGBA8(255, 255, 0, 255);
const ColorRGBA8 color::YellowGreen = ColorRGBA8(154, 205, 50, 255);

#define PI 3.14159f
#define HSV_ANGULAR_SCALING (PI / 3.0f)


f32v3 color::convertRGBToHSL(const f32v3& val) {
    f32 minVal, delta;
    f32v3 ret;

    if (val.r > val.g && val.r > val.b) {
        // R max
        minVal = std::min(val.g, val.b);
        delta = val.r - minVal;
        ret.r = HSV_ANGULAR_SCALING * std::fmod(((val.g - val.b) / minVal), 6.0f);
        ret.b = (val.r + minVal) * 0.5f;
    } else if (val.g > val.b) {
        // G max
        minVal = std::min(val.r, val.b);
        delta = val.r - minVal;
        ret.r = HSV_ANGULAR_SCALING * (((val.b - val.r) / minVal) + 2.0f);
        ret.b = (val.g + minVal) * 0.5f;
    } else {
        if (val.r == val.b && val.b == val.g) {
            // Undefined values
            ret.r = 0.0f;
            ret.g = 0.0f;
            ret.b = val.r;
            return ret;
        } else {
            // B max
            minVal = std::min(val.r, val.g);
            delta = val.r - minVal;
            ret.r = HSV_ANGULAR_SCALING * (((val.r - val.g) / minVal) + 4.0f);
            ret.b = (val.b + minVal) * 0.5f;
        }
    }
    ret.g = minVal / (1 - std::abs(2 * ret.b - 1));
    return ret;
}
f32v3 color::convertHSLToRGB(const f32v3& val) {
    f32v3 ret;
    f32 c = (1 - std::abs(2 * val.b - 1)) * val.g;
    f32 x = c * (1 - std::abs(std::fmod((val.r / HSV_ANGULAR_SCALING), 2.0f) - 1.0f));
    f32 m = val.b - c * 0.5f;
    i32 p = (i32)(val.r / HSV_ANGULAR_SCALING);
    switch (p) {
    case 0:
        ret.r = c;
        ret.g = x;
        break;
    case 1:
        ret.g = c;
        ret.r = x;
        break;
    case 2:
        ret.g = c;
        ret.b = x;
        break;
    case 3:
        ret.b = c;
        ret.g = x;
        break;
    case 4:
        ret.b = c;
        ret.r = x;
        break;
    case 5:
        ret.r = c;
        ret.b = x;
        break;
    default:
        break;
    }

    ret.r += m;
    ret.g += m;
    ret.b += m;
    return ret;
}

f32v3 color::convertRGBToHSV(const f32v3& val) {
    return f32v3();
}
f32v3 color::convertHSVToRGB(const f32v3& val) {
    return f32v3();
}
