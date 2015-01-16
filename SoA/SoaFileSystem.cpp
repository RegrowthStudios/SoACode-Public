#include "stdafx.h"
#include "SoaFileSystem.h"

void SoaFileSystem::init() {
    m_roots["Music"] = vio::IOManager("Data/Music");

}

const vio::IOManager& SoaFileSystem::get(const nString& name) const {
    return m_roots.at(name);
}

