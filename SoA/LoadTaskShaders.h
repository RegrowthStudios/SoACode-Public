#pragma once
#include <SDL\SDL.h>

#include "Shader.h"
#include "SpriteBatch.h"
#include "Errors.h"
#include "LoadMonitor.h"

//SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
//SDL_GL_CreateContext(scr->_game->getWindowHandle())

// Sample Dependency Task
class LoadTaskShaders : public ILoadTask {
public:
    LoadTaskShaders(SpriteBatch** sb, SDL_GLContext context, SDL_Window* window) :
        _sb(sb),
        _context(context),
        _window(window) {
        // Empty
    }
private:
    virtual void load() {
        SDL_GL_MakeCurrent(_window, _context);

        //(*_sb)->dispose();
        //*_sb = new SpriteBatch(true, true);

        textureShader.Initialize();
        basicColorShader.DeleteShader();
        basicColorShader.Initialize();
        simplexNoiseShader.DeleteShader();
        //simplexNoiseShader.Initialize();
        hdrShader.DeleteShader();
        motionBlurShader.DeleteShader();
        motionBlurShader.Initialize();
        hdrShader.Initialize();
        blockShader.DeleteShader();
        blockShader.Initialize();
        cutoutShader.DeleteShader();
        cutoutShader.Initialize();
        transparencyShader.DeleteShader();
        transparencyShader.Initialize();
        atmosphereToSkyShader.DeleteShader();
        atmosphereToSkyShader.Initialize();
        atmosphereToGroundShader.DeleteShader();
        atmosphereToGroundShader.Initialize();
        spaceToSkyShader.DeleteShader();
        spaceToSkyShader.Initialize();
        spaceToGroundShader.DeleteShader();
        spaceToGroundShader.Initialize();
        waterShader.DeleteShader();
        waterShader.Initialize();
        billboardShader.DeleteShader();
        billboardShader.Initialize();
        fixedSizeBillboardShader.DeleteShader();
        fixedSizeBillboardShader.Initialize();
        sonarShader.DeleteShader();
        sonarShader.Initialize();
        physicsBlockShader.DeleteShader();
        physicsBlockShader.Initialize();
        treeShader.DeleteShader();
        treeShader.Initialize();

        checkGlError("InitializeShaders()");

        SDL_GL_DeleteContext(_context);
    }

    SDL_GLContext _context;
    SDL_Window* _window;
    SpriteBatch** _sb;
};