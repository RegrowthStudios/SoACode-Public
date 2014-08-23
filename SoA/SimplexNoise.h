#pragma once

// Multi-octave Simplex noise
// For each octave, a higher frequency/lower amplitude function will be added to the original.
// The higher the persistence [0-1], the more of each succeeding octave will be added.

//SOURCE
// http://www.6by9.net/simplex-noise-for-c-and-python/

//4747
//5324
double ridged_octave_noise_2d( const double octaves, const double persistence, const double freq, const double x, const double y );

double ridged_octave_noise_3d_1(const double octaves,
                    const double persistence,
                    const double freq,
                    const double x,
                    const double y,
                    const double z);

double ridged_octave_noise_3d_2(const double octaves,
                    const double persistence,
                    const double freq,
                    const double x,
                    const double y,
                    const double z);

double reverse_ridged_octave_noise_2d( const double octaves, const double persistence, const double freq, const double x, const double y );

inline double octave_noise_2d(const double octaves,
                    const double persistence,
                    const double freq,
                    const double x,
                    const double y);
inline double octave_noise_3d(const double octaves,
                    const double persistence,
                    const double freq,
                    const double x,
                    const double y,
                    const double z);
double octave_noise_4d(const double octaves,
                    const double persistence,
                    const double freq,
                    const double x,
                    const double y,
                    const double z,
                    const double w);


// Scaled Multi-octave Simplex noise
// The result will be between the two parameters passed.
double scaled_ridged_octave_noise_2d(  const double octaves,
                            const double persistence,
                            const double freq,
                            const double loBound,
                            const double hiBound,
                            const double x,
                            const double y);
double scaled_reverse_ridged_octave_noise_2d(  const double octaves,
                            const double persistence,
                            const double freq,
                            const double loBound,
                            const double hiBound,
                            const double x,
                            const double y);
double scaled_octave_noise_2d(  const double octaves,
                            const double persistence,
                            const double freq,
                            const double loBound,
                            const double hiBound,
                            const double x,
                            const double y);
double scaled_ridged_octave_noise_3d(  const double octaves,
                            const double persistence,
                            const double freq,
                            const double loBound,
                            const double hiBound,
                            const double x,
                            const double y,
							const double z);
double scaled_reverse_ridged_octave_noise_3d(  const double octaves,
                            const double persistence,
                            const double freq,
                            const double loBound,
                            const double hiBound,
                            const double x,
                            const double y,
							const double z);
double scaled_octave_noise_3d(  const double octaves,
                            const double persistence,
                            const double freq,
                            const double loBound,
                            const double hiBound,
                            const double x,
                            const double y,
                            const double z);
double scaled_octave_noise_4d(  const double octaves,
                            const double persistence,
                            const double freq,
                            const double loBound,
                            const double hiBound,
                            const double x,
                            const double y,
                            const double z,
                            const double w);

// Scaled Raw Simplex noise
// The result will be between the two parameters passed.
inline double scaled_raw_noise_2d( const double loBound,
                        const double hiBound,
                        const double x,
                        const double y);
inline double scaled_raw_noise_3d( const double loBound,
                        const double hiBound,
                        const double x,
                        const double y,
                        const double z);
double scaled_raw_noise_4d( const double loBound,
                        const double hiBound,
                        const double x,
                        const double y,
                        const double z,
                        const double w);


// Raw Simplex noise - a single noise value.
inline double raw_noise_2d(const double x, const double y);
inline double raw_noise_3d(const double x, const double y, const double z);
double raw_noise_4d(const double x, const double y, const double, const double w);

inline double dot(const int* g, const double x, const double y);
inline double dot(const int* g, const double x, const double y, const double z);
inline double dot(const int* g, const double x, const double y, const double z, const double w);


// The gradients are the midpoints of the vertices of a cube.
static const int grad3[12][3] = {
    {1,1,0}, {-1,1,0}, {1,-1,0}, {-1,-1,0},
    {1,0,1}, {-1,0,1}, {1,0,-1}, {-1,0,-1},
    {0,1,1}, {0,-1,1}, {0,1,-1}, {0,-1,-1}
};


// The gradients are the midpoints of the vertices of a hypercube.
static const int grad4[32][4]= {
    {0,1,1,1},  {0,1,1,-1},  {0,1,-1,1},  {0,1,-1,-1},
    {0,-1,1,1}, {0,-1,1,-1}, {0,-1,-1,1}, {0,-1,-1,-1},
    {1,0,1,1},  {1,0,1,-1},  {1,0,-1,1},  {1,0,-1,-1},
    {-1,0,1,1}, {-1,0,1,-1}, {-1,0,-1,1}, {-1,0,-1,-1},
    {1,1,0,1},  {1,1,0,-1},  {1,-1,0,1},  {1,-1,0,-1},
    {-1,1,0,1}, {-1,1,0,-1}, {-1,-1,0,1}, {-1,-1,0,-1},
    {1,1,1,0},  {1,1,-1,0},  {1,-1,1,0},  {1,-1,-1,0},
    {-1,1,1,0}, {-1,1,-1,0}, {-1,-1,1,0}, {-1,-1,-1,0}
};


// Permutation table.  The same list is repeated twice.
static const int perm[512] = {
    151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
    8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
    35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
    134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
    55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208, 89,
    18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
    250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
    189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
    172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
    228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
    107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,

    151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
    8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
    35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
    134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
    55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208, 89,
    18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
    250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
    189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
    172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
    228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
    107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};


// A lookup table to traverse the simplex around a given point in 4D.
static const int simplex[64][4] = {
    {0,1,2,3},{0,1,3,2},{0,0,0,0},{0,2,3,1},{0,0,0,0},{0,0,0,0},{0,0,0,0},{1,2,3,0},
    {0,2,1,3},{0,0,0,0},{0,3,1,2},{0,3,2,1},{0,0,0,0},{0,0,0,0},{0,0,0,0},{1,3,2,0},
    {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
    {1,2,0,3},{0,0,0,0},{1,3,0,2},{0,0,0,0},{0,0,0,0},{0,0,0,0},{2,3,0,1},{2,3,1,0},
    {1,0,2,3},{1,0,3,2},{0,0,0,0},{0,0,0,0},{0,0,0,0},{2,0,3,1},{0,0,0,0},{2,1,3,0},
    {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
    {2,0,1,3},{0,0,0,0},{0,0,0,0},{0,0,0,0},{3,0,1,2},{3,0,2,1},{0,0,0,0},{3,1,2,0},
    {2,1,0,3},{0,0,0,0},{0,0,0,0},{0,0,0,0},{3,1,0,2},{0,0,0,0},{3,2,0,1},{3,2,1,0}
};

