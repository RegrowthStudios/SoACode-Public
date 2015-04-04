#include "stdafx.h"
#include "GasGiantComponentRenderer.h"

#include "ShaderLoader.h"
#include <Vorb/MeshGenerators.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/RasterizerState.h>
#include <Vorb/graphics/ShaderManager.h>

#define ICOSPHERE_SUBDIVISIONS 6

GasGiantComponentRenderer::GasGiantComponentRenderer() {
    // Empty
}

GasGiantComponentRenderer::~GasGiantComponentRenderer() {
    disposeShader();
    if (m_icoVbo) {
        vg::GpuMemory::freeBuffer(m_icoVbo);
    }
    if (m_icoIbo) {
        vg::GpuMemory::freeBuffer(m_icoIbo);
    }
}

void GasGiantComponentRenderer::draw(const GasGiantComponent& ggCmp, const f32m4& VP, const f32v3& relCamPos, const f32v3& lightDir, const SpaceLightComponent* spComponent) {
    // Lazily construct buffer and shaders
    if (!m_program) {
        m_program = ShaderLoader::createProgramFromFile("Shaders/GasGiant/GasGiant.vert",
                                                        "Shaders/GasGiant/GasGiant.frag");
    }
    if (!m_icoVbo) buildMesh();
}

void GasGiantComponentRenderer::disposeShader() {

}

void GasGiantComponentRenderer::buildShaders() {

}

void GasGiantComponentRenderer::buildMesh() {
    std::vector<ui32> indices;
    std::vector<f32v3> positions;

    // TODO(Ben): Optimize with LOD for far viewing
    vmesh::generateIcosphereMesh(ICOSPHERE_SUBDIVISIONS, indices, positions);
    m_numIndices = indices.size();

    vg::GpuMemory::createBuffer(m_icoVbo);
    vg::GpuMemory::createBuffer(m_icoIbo);

    vg::GpuMemory::bindBuffer(m_icoVbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_icoVbo, vg::BufferTarget::ARRAY_BUFFER, positions.size() * sizeof(f32v3),
                                    positions.data(), vg::BufferUsageHint::STATIC_DRAW);
    vg::GpuMemory::bindBuffer(0, vg::BufferTarget::ARRAY_BUFFER);

    vg::GpuMemory::bindBuffer(m_icoIbo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_icoIbo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(ui32),
                                    indices.data(), vg::BufferUsageHint::STATIC_DRAW);
    vg::GpuMemory::bindBuffer(0, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
}
