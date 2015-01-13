#pragma once
#include <SDL/SDL.h>

#include <Vorb/RPC.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/VorbPreDecl.inl>
#include <Vorb/graphics/SpriteBatch.h>

#include "Errors.h"
#include "LoadMonitor.h"

DECL_VG(class, GLProgramManager);
DECL_VIO(class, IOManager);

class OnReloadShaderKeyDown;

class ProgramGenDelegate : public IDelegate<void*> {
public:
    virtual void invoke(Sender sender, void* userData) override;

    nString name;
    vg::ShaderSource vs;
    vg::ShaderSource fs;
    std::vector<nString>* attr = nullptr;

    vcore::RPC rpc;

    vg::GLProgram* program = nullptr;
    nString errorMessage;
};

#define APPROXIMATE_NUM_SHADERS_LOADING 100

// TODO(Ben): Somehow make this asynchronous
class LoadTaskShaders : public ILoadTask {
    // So that these classes can call load()
    friend class LoadScreen;
    friend class GamePlayScreen;
    friend class OnReloadShadersKeyDown;
    friend class OnMainMenuReloadShadersKeyDown;
    friend class MainMenuScreen;
public:
    LoadTaskShaders(vcore::RPCManager* glrpc, vg::GLProgramManager* glProgramManager) :
        m_glrpc(glrpc),
        m_glProgramManager(glProgramManager) {
        // Empty
    }
    virtual void load() override;
private:
    vg::ShaderSource createShaderCode(const vg::ShaderType& stage, const vio::IOManager& iom, const cString path, const cString defines = nullptr);
    ProgramGenDelegate* createProgram(nString name, const vg::ShaderSource& vs, const vg::ShaderSource& fs, std::vector<nString>* attr = nullptr);

    vcore::RPCManager* m_glrpc = nullptr;
    ProgramGenDelegate m_generators[APPROXIMATE_NUM_SHADERS_LOADING];
    size_t m_numGenerators = 0;
    std::vector<const cString> m_filesToDelete;
    vg::GLProgramManager* m_glProgramManager = nullptr;
};