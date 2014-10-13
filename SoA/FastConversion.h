// FastConversion
//
// Class designed to use SSE when possible for:
//   fast flooring
//   fast ceiling
//   fast rounding toward zero
//   fast rounding away from zero
//
// NOTE:  This class is only intended for use with the following
//        conversions...
//        f64 -> f64
//        f64 -> f32
//        f64 -> i64
//        f64 -> i32
//        f32 -> f64
//        f32 -> f32
//        f32 -> i64
//        f32 -> i32
//
#pragma once
#include <immintrin.h>
#include <cmath>

#include "types.h"


static const f32 negOneHalf = -0.5f;


template<class T, class U>
class FastConversion {
    public:
        FastConversion() {
            // empty
        }

        // rounds to the next lowest whole number
        //     1.5 -->  1.0
        //    -1.5 --> -2.0
        inline U floor(const T& x) {
            U output = 0;
            // slowest version
            #if defined(__APPLE__) || defined(__linux__) || defined(WT64)
                // std::floor expects a double
                output = static_cast<U>(std::floor(static_cast<double>(x)));
            #elif defined(WT32)
                __asm {
                    fld x;
                    fadd st, st(0);
                    fadd negOneHalf;
                    fistp i;
                    sar i, 1;
                };
            #else
                // std::floor expects a double
                output = static_cast<U>(std::floor(static_cast<double>(x)));
            #endif
            return output;
        }

        // rounds to the next highest whole number
        //     1.5 -->  2.0
        //    -1.5 --> -1.0
        inline U ceiling(const T& x) {
            U output = 0;
            #if defined(__APPLE__) || defined(__linux__) || defined(WT64)
                // std::ceil expects a double
                output = static_cast<U>(std::ceil(static_cast<double>(x)));
            #elif defined(WT32)
                __asm {
                    fld x;
                    fadd st, st(0);
                    fsubr negOneHalf;
                    fistp i;
                    sar i, 1;
                };
            #else
                // std::ceil expects a double
                output = static_cast<U>(std::ceil(static_cast<double>(x)));
            #endif
            return output;
        }

        // rounds towards zero
        //     1.5 -->  1.0
        //    -1.5 --> -1.0
        inline U trunc(const T& x) {
            U output = 0;
            #if defined(__APPLE__) || defined(__linux__) || defined(WT64)
                // std::trunc expects a double
                output = static_cast<U>(std::trunc(static_cast<double>(x)));
            #elif defined(WT32)
                // std::trunc expects a double
                output = static_cast<U>(std::trunc(static_cast<double>(x)));
            #else
                // std::trunc expects a double
                output = static_cast<U>(std::trunc(static_cast<double>(x)));
            #endif
            return output;
        }

        // round to nearest
        //     1.5 -->  2.0
        //     1.4 -->  1.0
        //    -1.5 --> -2.0
        //    -1.4 --> -1.0
        inline U round(const T& x) {
            U output = 0;
            #if defined(__APPLE__) || defined(__linux__) || defined(WT64)
                // std::round expects a double
                output = static_cast<U>(std::round(static_cast<double>(x)));
            #elif defined(WT32)
                // std::round expects a double
                output = static_cast<U>(std::round(static_cast<double>(x)));
            #else
                // std::round expects a double
                output = static_cast<U>(std::round(static_cast<double>(x)));
            #endif
            return output;
        }
};