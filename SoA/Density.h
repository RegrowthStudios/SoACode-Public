#ifndef		HAS_DENSITY_H_BEEN_INCLUDED
#define		HAS_DENSITY_H_BEEN_INCLUDED

class VoxelMatrix;

extern const VoxelMatrix* gMatrix;

float Density_Func(const f32v3& worldPosition);

#endif	//	HAS_DENSITY_H_BEEN_INCLUDED