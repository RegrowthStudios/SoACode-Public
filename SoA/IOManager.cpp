#include "stdafx.h"
#include "IOManager.h"

#include "utils.h"

IOManager::IOManager() :
    _searchDir(nullptr) {
    boost::filesystem::path path;
    boost::filesystem::path absPath;

    if (!_execDir) {
        // Find The Executing Path
        path = boost::filesystem::initial_path();
        absPath = boost::filesystem::system_complete(path);
        nString exeDir;
        convertWToMBString((cwString)absPath.c_str(), exeDir);
        setExecutableDirectory(exeDir);
    }

    if (!_cwDir) {
        // Find The CWD
        path = boost::filesystem::current_path();
        absPath = boost::filesystem::system_complete(path);
        nString cwDir;
        convertWToMBString((cwString)absPath.c_str(), cwDir);
        setCurrentWorkingDirectory(cwDir);
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
void IOManager::setCurrentWorkingDirectory(const nString& s) {
    // Delete The Old CWD
    if (_cwDir) free(_cwDir);

    if (s.empty()) {
        // No Search Directory Now
        _cwDir = nullptr;
        return;
    } else {
        // Copy The String
        i32 l = s.length();
        _cwDir = (cString)malloc(l + 1);
        strcpy_s(_cwDir, l + 1, s.data());
        _cwDir[l] = 0;
    }
}
void IOManager::setExecutableDirectory(const nString& s) {
    // Delete The Old Executing Directory
    if (_execDir) free(_execDir);

    if (s.empty()) {
        // No Search Directory Now
        _execDir = nullptr;
        return;
    } else {
        // Copy The String
        i32 l = s.length();
        _execDir = (cString)malloc(l + 1);
        strcpy_s(_execDir, l + 1, s.data());
        _execDir[l] = 0;
    }
}

void IOManager::getDirectory(const cString path, nString& resultPath) {
    boost::filesystem::path p(path);
    if (!boost::filesystem::is_directory(p)) {
        p = p.remove_filename();
    }
    if (!p.is_absolute()) {
        p = boost::filesystem::system_complete(p);
    }
    convertWToMBString((cwString)p.c_str(), resultPath);
}

void IOManager::getDirectoryEntries(nString dirPath, std::vector<boost::filesystem::path>& entries) {
    boost::filesystem::directory_iterator end_iter;
    boost::filesystem::path targetDir(dirPath);

    if (boost::filesystem::exists(targetDir)) {
        for (boost::filesystem::directory_iterator dir_iter(targetDir); dir_iter != end_iter; ++dir_iter) {
            // Emplace a copy of the path reference
            entries.emplace_back(boost::filesystem::path(dir_iter->path()));
        }
    }
}

FILE* IOManager::openFile(const cString path, const cString flags) {
   nString filePath;

    if (resolveFile(path, filePath)) {
        FILE* f;
        errno_t err = fopen_s(&f, filePath.data(), flags);
        return err == 0 ? f : nullptr;
    }

    // TODO: Implement
    return nullptr;
}

bool IOManager::readFileToString(const cString path, nString& data) {
    nString filePath;

    if (resolveFile(path, filePath)) {
        std::ifstream t(filePath, std::ios::in | std::ios::binary);

        t.seekg(0, std::ios::end);
        i32 length = t.tellg();
        t.seekg(0, std::ios::beg);
        length -= t.tellg();

        data.resize(length + 1);
        t.read(&(data[0]), length);
        t.close();

        data[length] = '\0';
        return true;
    }
    return false;
}
const cString IOManager::readFileToString(const cString path) {
    nString filePath;

    if (resolveFile(path, filePath)) {
        std::ifstream t(filePath, std::ios::in | std::ios::binary);

        t.seekg(0, std::ios::end);
        i32 length = t.tellg();
        t.seekg(0, std::ios::beg);
        length -= t.tellg();

        cString data = new char[length + 1];
        t.read(data, length);
        t.close();

        data[length] = '\0';
        return data;
    }
    return nullptr;
}
bool IOManager::readFileToData(const cString path, std::vector<ui8>& data) {
    nString filePath;

    if (resolveFile(path, filePath)) {
        std::ifstream t(filePath, std::ios::in | std::ios::binary);

        t.seekg(0, std::ios::end);
        i32 length = t.tellg();
        t.seekg(0, std::ios::beg);

        data.resize(length);
        t.read((char*)&(data[0]), length);
        t.close();

        return true;
    }
    return false;
}

bool IOManager::resolveFile(const cString path, nString& resultAbsolutePath) {
    boost::filesystem::path p(path);
    if (p.is_absolute()) {
        if (boost::filesystem::exists(p)) {
            convertWToMBString((cwString)p.c_str(), resultAbsolutePath);
            return true;
        }
    }

    boost::filesystem::path concatPath;
    if (_searchDir) {
        concatPath = boost::filesystem::path(_searchDir);
        concatPath /= p;
        if (boost::filesystem::exists(concatPath)) {
            convertWToMBString((cwString)concatPath.c_str(), resultAbsolutePath);
            return true;
        }
    }

    if (_cwDir) {
        concatPath = boost::filesystem::path(_cwDir);
        concatPath /= p;
        if (boost::filesystem::exists(concatPath)) {
            convertWToMBString((cwString)concatPath.c_str(), resultAbsolutePath);
            return true;
        }
    }

    if (_execDir) {
        concatPath = boost::filesystem::path(_execDir);
        concatPath /= p;
        if (boost::filesystem::exists(concatPath)) {
            convertWToMBString((cwString)concatPath.c_str(), resultAbsolutePath);
            return true;
        }
    }

    return false;
}

bool IOManager::writeStringToFile(const cString path, const nString& data) {
    FILE* file = nullptr;
    fopen_s(&file, path, "w");
    if (file) {
      fwrite(data.c_str(), 1, data.length(), file);
      fclose(file);
      return true;
    }
    return false;
}

bool IOManager::makeDirectory(const cString path) {
    return boost::filesystem::create_directory(path);
}

bool IOManager::fileExists(const cString path) {
    boost::filesystem::path p(path);
    return !boost::filesystem::is_directory(p) && boost::filesystem::exists((p));
}

bool IOManager::directoryExists(const cString path) {
    boost::filesystem::path p(path);
    return boost::filesystem::is_directory(p) && boost::filesystem::exists((p));
}

cString IOManager::_execDir = nullptr;
cString IOManager::_cwDir = nullptr;
