#include "stdafx.h"
#include "IOManager.h"

#include <boost\filesystem.hpp>

#include "utils.h"

IOManager::IOManager() :
_searchDir(nullptr) {
    boost::filesystem::path path;
    boost::filesystem::path absPath;

    if (!_execDir) {
        // Find The Executing Path
        path = boost::filesystem::initial_path();
        absPath = boost::filesystem::system_complete(path);
        setExecutableDirectory(convertWToMBString((cwString)absPath.c_str()));
    }

    if (!_cwDir) {
        // Find The CWD
        path = boost::filesystem::current_path();
        absPath = boost::filesystem::system_complete(path);
        setCurrentWorkingDirectory(convertWToMBString((cwString)absPath.c_str()));
    }

    // Search Directory Defaults To CWD
    setSearchDirectory(_cwDir);
}
IOManager::~IOManager() {
    if (_searchDir) {
        free(_searchDir);
        _searchDir = nullptr;
    }
    if (_cwDir) {
        free(_cwDir);
        _cwDir = nullptr;
    }
    if (_execDir) {
        free(_execDir);
        _execDir = nullptr;
    }
}

void IOManager::setSearchDirectory(const cString s) {
    // Delete The Old Search Directory
    if (_searchDir) free(_searchDir);

    if (!s) {
        // No Search Directory Now
        _searchDir = nullptr;
        return;
    } else {
        // Copy The String
        i32 l = strlen(s);
        _searchDir = (cString)malloc(l + 1);
        strcpy_s(_searchDir, l + 1, s);
        _searchDir[l] = 0;
    }
}
void IOManager::setCurrentWorkingDirectory(const cString s) {
    // Delete The Old CWD
    if (_cwDir) free(_cwDir);

    if (!s) {
        // No Search Directory Now
        _cwDir = nullptr;
        return;
    } else {
        // Copy The String
        i32 l = strlen(s);
        _cwDir = (cString)malloc(l + 1);
        strcpy_s(_cwDir, l + 1, s);
        _cwDir[l] = 0;
    }
}
void IOManager::setExecutableDirectory(const cString s) {
    // Delete The Old Executing Directory
    if (_execDir) free(_execDir);

    if (!s) {
        // No Search Directory Now
        _execDir = nullptr;
        return;
    } else {
        // Copy The String
        i32 l = strlen(s);
        _execDir = (cString)malloc(l + 1);
        strcpy_s(_execDir, l + 1, s);
        _execDir[l] = 0;
    }
}

const cString IOManager::getDirectory(const cString path) {
    boost::filesystem::path p(path);
    if (!boost::filesystem::is_directory(p)) {
        p = p.remove_filename();
    }
    if (!p.is_absolute()) {
        p = boost::filesystem::system_complete(p);
    }
    return convertWToMBString((cwString)p.c_str());
}

FILE* IOManager::openFile(const cString path, const cString flags) {
    const cString filePath = resolveFile(path);

    if (filePath) {
        FILE* f;
        errno_t err = fopen_s(&f, filePath, flags);
        delete[] filePath;
        return err == 0 ? f : nullptr;
    }

    // TODO: Implement
    return nullptr;
}

const cString IOManager::readFileToString(const cString path) {
    const cString filePath = resolveFile(path);

    if (filePath) {
        std::ifstream t(filePath, std::ios::in | std::ios::binary);
        delete[] filePath;

        t.seekg(0, std::ios::end);
        i32 length = t.tellg();
        t.seekg(0, std::ios::beg);
        length -= t.tellg();

        cString buffer = new char[length + 1];
        t.read(buffer, length);
        t.close();

        buffer[length] = 0;
        return buffer;
    }
}
const cString IOManager::readFileToData(const cString path, i32* len) {
    const cString filePath = resolveFile(path);

    if (filePath) {
        std::ifstream t(filePath, std::ios::in | std::ios::binary);
        delete[] filePath;

        t.seekg(0, std::ios::end);
        i32 length = t.tellg();
        t.seekg(0, std::ios::beg);
        length -= t.tellg();
        if (len) *len = length;

        cString buffer = new char[length];
        t.read(buffer, length);
        t.close();

        return buffer;
    }
}

const cString IOManager::resolveFile(const cString path) {
    boost::filesystem::path p(path);
    if (p.is_absolute()) {
        if (boost::filesystem::exists(p)) {
            return convertWToMBString((cwString)p.c_str());
        }
    }

    boost::filesystem::path concatPath;
    if (_searchDir) {
        concatPath = boost::filesystem::path(_searchDir);
        concatPath /= p;
        if (boost::filesystem::exists(concatPath)) {
            return convertWToMBString((cwString)concatPath.c_str());
        }
    }
    if (_cwDir) {
        concatPath = boost::filesystem::path(_cwDir);
        concatPath /= p;
        if (boost::filesystem::exists(concatPath)) {
            return convertWToMBString((cwString)concatPath.c_str());
        }
    }
    if (_execDir) {
        concatPath = boost::filesystem::path(_execDir);
        concatPath /= p;
        if (boost::filesystem::exists(concatPath)) {
            return convertWToMBString((cwString)concatPath.c_str());
        }
    }

    return nullptr;
}

cString IOManager::_execDir = nullptr;
cString IOManager::_cwDir = nullptr;
