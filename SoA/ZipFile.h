#pragma once

#include <minizip/unzip.h>
#include <Vorb/types.h>

class ZipFile {
public:
    ZipFile(nString fileName);
    ~ZipFile();

    ui8* readFile(nString fileName, size_t& fileSize);
    bool isFailure() {
        return _failure;
    }
private:
    unzFile _zipFile;
    unz_global_info _globalInfo;
    bool _failure;
};
