#include "stdafx.h"
#include "ModPathResolver.h"

void ModPathResolver::init(const vio::Path& defaultPath, const vio::Path& modPath) {
    setDefaultDir(defaultPath);
    setModDir(modPath);
}

void ModPathResolver::setDefaultDir(const vio::Path& path) {
    defaultIom.setSearchDirectory(path);
}

void ModPathResolver::setModDir(const vio::Path& path) {
    modIom.setSearchDirectory(path);
}

bool ModPathResolver::resolvePath(const vio::Path& path, vio::Path& resultAbsolutePath, bool printModNotFound /* = false */) const {
    if (!modIom.resolvePath(path, resultAbsolutePath)) {
        if (printModNotFound) {
            printf("Did not find path %s in %s.", path.getCString(), modIom.getSearchDirectory().getCString());
        }
        if (!defaultIom.resolvePath(path, resultAbsolutePath)) {
            if (printModNotFound) {
                printf("Did not find path %s in %s.", path.getCString(), defaultIom.getSearchDirectory().getCString());
            }
            return false;
        }
    }
    return true;
}
