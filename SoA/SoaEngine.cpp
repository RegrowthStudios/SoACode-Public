#include "stdafx.h"
#include "SoaEngine.h"

#include "SoaState.h"
#include "GLProgramManager.h"
#include "DebugRenderer.h"
#include "MeshManager.h"

bool SoaEngine::initState(OUT SoaState* state) {
    state->glProgramManager = std::make_unique<vg::GLProgramManager>();
    state->debugRenderer = std::make_unique<DebugRenderer>(state->glProgramManager.get());
    state->meshManager = std::make_unique<MeshManager>(state->glProgramManager.get());
    return true;
}

bool SoaEngine::loadSpaceSystem(OUT SoaState* state, const SpaceSystemLoadData& loadData) {
    AutoDelegatePool pool;
    vpath path = "SoASpace.log";
    vfile file;
    path.asFile(&file);
    vfstream fs = file.open(vio::FileOpenFlags::READ_WRITE_CREATE);
    pool.addAutoHook(&state->spaceSystem.onEntityAdded, [=] (Sender, vcore::EntityID eid) {
        fs.write("Entity added: %d\n", eid);
    });
    for (auto namedTable : state->spaceSystem.getComponents()) {
        auto table = state->spaceSystem.getComponentTable(namedTable.first);
        pool.addAutoHook(&table->onEntityAdded, [=] (Sender, vcore::ComponentID cid, vcore::EntityID eid) {
            fs.write("Component \"%s\" added: %d -> Entity %d\n", namedTable.first.c_str(), cid, eid);
        });
    }

    state->spaceSystem.init(state->glProgramManager.get());
    state->spaceSystem.addSolarSystem(loadData.filePath);

    pool.dispose();
    return true;
}

bool SoaEngine::loadGameSystem(OUT SoaState* state, const GameSystemLoadData& loadData) {
    return true;
}

void SoaEngine::destroyAll(OUT SoaState* state) {
    destroyGameSystem(state);
    destroySpaceSystem(state);
}

void SoaEngine::destroyGameSystem(OUT SoaState* state) {

}

void SoaEngine::destroySpaceSystem(OUT SoaState* state) {

}
