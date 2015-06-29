///
/// CpuNoise.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 8 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Collection of Cpu Noise functions
///

#pragma once

#ifndef CpuNoise_h__
#define CpuNoise_h__

namespace CpuNoise {
    extern f64 rawAshimaSimplex3D(const f64v3& v);
    extern f64v2 cellular(const f64v3& P);
}

#endif // CpuNoise_h__
