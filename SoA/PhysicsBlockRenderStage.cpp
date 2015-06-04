#include "stdafx.h"
#include "PhysicsBlockRenderStage.h"

#include "Camera.h"
#include "ChunkRenderer.h"
#include "PhysicsBlocks.h"
#include "GameRenderParams.h"
#include "SoaOptions.h"

PhysicsBlockRenderStage::PhysicsBlockRenderStage(GameRenderParams* gameRenderParams, 
                                                 const std::vector<PhysicsBlockMesh*>& physicsBlockMeshes,
                                                 vg::GLProgram* glProgram) :
    _gameRenderParams(gameRenderParams),
    _physicsBlockMeshes(physicsBlockMeshes),
    _glProgram(glProgram) {
    // Empty
}

void PhysicsBlockRenderStage::render(const Camera* camera) {

    _glProgram->use();

    glUniform1f(_glProgram->getUniform("lightType"), _gameRenderParams->lightActive);

    glUniform1f(_glProgram->getUniform("alphaMult"), 1.0f);

    glUniform3fv(_glProgram->getUniform("eyeNormalWorldspace"), 1, &(_gameRenderParams->chunkCamera->getDirection()[0]));
    glUniform1f(_glProgram->getUniform("fogEnd"), _gameRenderParams->fogEnd);
    glUniform1f(_glProgram->getUniform("fogStart"), _gameRenderParams->fogStart);
    glUniform3fv(_glProgram->getUniform("fogColor"), 1, &(_gameRenderParams->fogColor[0]));
    glUniform3fv(_glProgram->getUniform("lightPosition_worldspace"), 1, &(_gameRenderParams->sunlightDirection[0]));
    glUniform1f(_glProgram->getUniform("specularExponent"), soaOptions.get(OPT_SPECULAR_EXPONENT).value.f);
    glUniform1f(_glProgram->getUniform("specularIntensity"), soaOptions.get(OPT_SPECULAR_INTENSITY).value.f * 0.3f);

    glUniform1f(_glProgram->getUniform("sunVal"), _gameRenderParams->sunlightIntensity);

    float blockAmbient = 0.000f;
    glUniform3f(_glProgram->getUniform("ambientLight"), blockAmbient, blockAmbient, blockAmbient);
    glUniform3fv(_glProgram->getUniform("lightColor"), 1, &(_gameRenderParams->sunlightColor[0]));

    if (NoChunkFade) {
        glUniform1f(_glProgram->getUniform("fadeDistance"), (GLfloat)10000.0f);
    } else {
        glUniform1f(_glProgram->getUniform("fadeDistance"), (GLfloat)soaOptions.get(OPT_VOXEL_RENDER_DISTANCE).value.f - 12.5f);
    }

    for (Uint32 i = 0; i < _physicsBlockMeshes.size(); i++) {
        PhysicsBlockBatch::draw(_physicsBlockMeshes[i], _glProgram, _gameRenderParams->chunkCamera->getPosition(),
                                _gameRenderParams->chunkCamera->getViewProjectionMatrix());
    }

    _glProgram->unuse();
}

