#include "stdafx.h"
#include "ModPathResolver.h"


ModPathResolver::ModPathResolver() {
}


ModPathResolver::~ModPathResolver() {
}

void ModPathResolver::init(const vio::Path& defaultPath, const vio::Path& modPath) {

}

void ModPathResolver::setDefaultDir(const vio::Path& path) {

}

void ModPathResolver::setModDir(const vio::Path& path) {

}

bool ModPathResolver::resolvePath(const vio::Path& path, vio::Path& resultAbsolutePath, bool printModNotFound /* = false */) {
    if (!modIom.resolvePath(path, resultAbsolutePath)) {
        if (printModNotFound) {
            printf("Did not find path %s in %s.", path.getCString(), modIom.getSearchDirectory());
        }
        if (!defaultIom.resolvePath(path, resultAbsolutePath)) {
            if (printModNotFound) {
                printf("Did not find path %s in %s.", path.getCString(), defaultIom.getSearchDirectory());
            }
            return false;
        }
    }
    return true;
}
