#ifndef		HAS_DENSITY_H_BEEN_INCLUDED
#define		HAS_DENSITY_H_BEEN_INCLUDED

#include <glm\glm.hpp>

class VoxelMatrix;

extern const VoxelMatrix* gMatrix;

float Density_Func(const glm::vec3& worldPosition);

#endif	//	HAS_DENSITY_H_BEEN_INCLUDED