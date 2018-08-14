#include "stdafx.h"
#include "PhysicsBlockRenderStage.h"

#include "Camera.h"
#include "ChunkRenderer.h"
#include "GameRenderParams.h"
#include "SoaOptions.h"

// TODO: Do we still want this as is? If so, reimplement and remove VORB_UNUSED tags.

PhysicsBlockRenderStage::PhysicsBlockRenderStage(GameRenderParams* gameRenderParams VORB_UNUSED, 
                                                 const std::vector<PhysicsBlockMesh*>& physicsBlockMeshes VORB_UNUSED,
                                                 vg::GLProgram* glProgram VORB_UNUSED) //:
//    _gameRenderParams(gameRenderParams),
//    _physicsBlockMeshes(physicsBlockMeshes),
//    _glProgram(glProgram) 
{
    // Empty
}

void PhysicsBlockRenderStage::render(const Camera* camera VORB_UNUSED) {

    /* _glProgram->use();

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

     glUniform1f(_glProgram->getUniform("fadeDistance"), (GLfloat)soaOptions.get(OPT_VOXEL_RENDER_DISTANCE).value.f - 12.5f);

     for (size_t i = 0; i < _physicsBlockMeshes.size(); i++) {
     PhysicsBlockBatch::draw(_physicsBlockMeshes[i], _glProgram, _gameRenderParams->chunkCamera->getPosition(),
     _gameRenderParams->chunkCamera->getViewProjectionMatrix());
     }

     _glProgram->unuse();*/
}

