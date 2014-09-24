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


template<typename IN, typename OUT>
class FastConversion {
    public:
        FastConversion() {}

        // rounds to the next lowest whole number
        //     1.5 -->  1.0
        //    -1.5 --> -2.0
        inline OUT floor(const IN& x) {
            OUT output = 0;
            // slowest version
            #if defined(__APPLE__) || defined(__linux__) || defined(WIN64)
                output = static_cast<OUT>(std::floor(static_cast<double>(x)));
            #elif defined(WIN32)
                __asm {
                    fld x;
                    fadd st, st(0);
                    fadd negOneHalf;
                    fistp i;
                    sar i, 1;
                };
            #else
                output = static_cast<OUT>(std::floor(static_cast<double>(x)));
            #endif
            return output;
        }

        // rounds to the next highest whole number
        //     1.5 -->  2.0
        //    -1.5 --> -1.0
        inline OUT ceiling(const IN& x) {
            OUT output = 0;
            #if defined(__APPLE__) || defined(__linux__) || defined(WIN64)
                output = static_cast<OUT>(std::ceil(static_cast<double>(x)));
            #elif defined(WIN32)
                __asm {
                    fld x;
                    fadd st, st(0);
                    fsubr negOneHalf;
                    fistp i;
                    sar i, 1;
                };
            #else
                output = static_cast<OUT>(std::ceil(static_cast<double>(x)));
            #endif
            return output;
        }

        // rounds towards zero
        //     1.5 -->  1.0
        //    -1.5 --> -1.0
        inline OUT trunc(const IN& x) {
            OUT output = 0;
            #if defined(__APPLE__) || defined(__linux__) || defined(WIN64)
                output = static_cast<OUT>(std::trunc(static_cast<double>(x)));
            #elif defined(WIN32)
                output = static_cast<OUT>(std::trunc(static_cast<double>(x)));
            #else
                output = static_cast<OUT>(std::trunc(static_cast<double>(x)));
            #endif
            return output;
        }

        // round to nearest
        //     1.5 -->  2.0
        //     1.4 -->  1.0
        //    -1.5 --> -2.0
        //    -1.4 --> -1.0
        inline OUT round(const IN& x) {
            OUT output = 0;
            #if defined(__APPLE__) || defined(__linux__) || defined(WIN64)
                output = static_cast<OUT>(std::round(static_cast<double>(x)));
            #elif defined(WIN32)
                output = static_cast<OUT>(std::round(static_cast<double>(x)));
            #else
                output = static_cast<OUT>(std::round(static_cast<double>(x)));
            #endif
            return output;
        }
};