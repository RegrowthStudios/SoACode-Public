#pragma once

#ifndef TestGasGiantScreen_h__
#define TestGasGiantScreen_h__

#include <Vorb/Events.hpp>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/ui/IGameScreen.h>
#include <Vorb/VorbPreDecl.inl>
#include <Vorb/graphics/TextureCache.h>

#include <vector>

class GasGiantRenderer;

DECL_VG(class, GLProgramManager);
DECL_VIO(class, IOManager);

class TestGasGiantScreen : public IGameScreen {
public:
    /************************************************************************/
    /* IGameScreen functionality                                            */
    /************************************************************************/
    virtual i32 getNextScreen() const override;
    virtual i32 getPreviousScreen() const override;
    virtual void build() override;
    virtual void destroy(const GameTime& gameTime) override;
    virtual void onEntry(const GameTime& gameTime) override;
    virtual void onExit(const GameTime& gameTime) override;
    virtual void onEvent(const SDL_Event& e) override;
    virtual void update(const GameTime& gameTime) override;
    virtual void draw(const GameTime& gameTime) override;

private:
    vg::GLProgramManager* m_glProgramManager;
    GasGiantRenderer* m_gasGiantRenderer;
    f32v3 m_eyePos;
    vg::TextureCache m_textureCache;

    std::vector<const cString> m_filesToDelete;

    vg::ShaderSource createShaderCode(const vg::ShaderType& stage, const vio::IOManager& iom, const cString path, const cString defines = nullptr);
};

#endif