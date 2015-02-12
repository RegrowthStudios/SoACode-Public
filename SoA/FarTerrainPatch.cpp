#include "stdafx.h"
#include "FarTerrainPatch.h"

FarTerrainMesh::~FarTerrainMesh() {
}

void FarTerrainMesh::recycleNormalMap(vg::TextureRecycler* recycler) {

}

void FarTerrainMesh::draw(const f64v3& relativePos, const Camera* camera, vg::GLProgram* program) const {

}

void FarTerrainMesh::drawWater(const f64v3& relativePos, const Camera* camera, vg::GLProgram* program) const {

}

void FarTerrainMesh::getClosestPoint(const f32v3& camPos, OUT f32v3& point) const {

}

void FarTerrainMesh::getClosestPoint(const f64v3& camPos, OUT f64v3& point) const {

}

FarTerrainPatch::~FarTerrainPatch() {

}

void FarTerrainPatch::init(const f64v2& gridPosition, WorldCubeFace cubeFace, int lod, const SphericalTerrainData* sphericalTerrainData, f64 width, TerrainRpcDispatcher* dispatcher) {

}

void FarTerrainPatch::update(const f64v3& cameraPos) {

}

void FarTerrainPatch::destroy() {

}

bool FarTerrainPatch::isRenderable() const {

}

bool FarTerrainPatch::isOverHorizon(const f32v3 &relCamPos, const f32v3 &point, f32 planetRadius) {

}

bool FarTerrainPatch::isOverHorizon(const f64v3 &relCamPos, const f64v3 &point, f64 planetRadius) {

}

void FarTerrainPatch::requestMesh() {

}
