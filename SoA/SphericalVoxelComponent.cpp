#include "stdafx.h"
#include "SphericalVoxelComponent.h"
#include "SphericalTerrainPatch.h"

#include <Vorb/IOManager.h>

SphericalVoxelComponent::SphericalVoxelComponent() {
    
}

SphericalVoxelComponent::~SphericalVoxelComponent() {

}

void SphericalVoxelComponent::init(const SphericalTerrainData* sphericalTerrainData, const IOManager* saveFileIom) {
    m_sphericalTerrainData = sphericalTerrainData;
    m_saveFileIom = saveFileIom;
}

void SphericalVoxelComponent::initVoxels(const glm::dvec3 &gpos, vvox::VoxelMapData* startingMapData, GLuint flags) {
    m_enabled = true;
}

void SphericalVoxelComponent::update(const Camera* voxelCamera) {

}

void SphericalVoxelComponent::getClosestChunks(glm::dvec3 &coord, Chunk **chunks) {

}

void SphericalVoxelComponent::endSession() {

}

void SphericalVoxelComponent::destroyVoxels() {
    m_enabled = false;
}

void SphericalVoxelComponent::updatePhysics(const Camera* camera) {

}
