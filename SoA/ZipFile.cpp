#include "stdafx.h"
#include "ZipFile.h"

#include "Errors.h"

ZipFile::ZipFile(nString fileName) : _zipFile(NULL), _failure(0) {
    _zipFile = unzOpen(fileName.c_str());
    if (_zipFile == NULL) {
        _failure = 1;
        return;
    }

    if (unzGetGlobalInfo(_zipFile, &_globalInfo) != UNZ_OK) {
        _failure = 1;
        pError(("could not read file global info in " + fileName).c_str());
        unzClose(_zipFile);
        return;
    }
}

ZipFile::~ZipFile() {
    if (_zipFile != NULL) {
        unzClose(_zipFile);
    }
}

ui8* ZipFile::readFile(nString fileName, size_t& fileSize) {
    const int READ_SIZE = 8196;
    const int MAX_FILENAME = 256;
    const char dir_delimiter = '/';

    unzGoToFirstFile(_zipFile);

    // Loop to extract all files
    uLong i;
    for (i = 0; i < _globalInfo.number_entry; ++i) {
        // Get info about current file.
        unz_file_info file_info;
        char filename[MAX_FILENAME];
        if (unzGetCurrentFileInfo(
            _zipFile,
            &file_info,
            filename,
            MAX_FILENAME,
            nullptr, 0, nullptr, 0) != UNZ_OK) {
            printf("could not read file info\n");
            return nullptr;
        }

        // Check if this entry is a directory or file.
        const size_t filename_length = strlen(filename);
        if (filename[filename_length - 1] != dir_delimiter && fileName.size() >= filename_length && fileName == &(filename[filename_length - fileName.size()])) //check that its not a dir and check that it is this file.
        {
            unsigned char *buffer = new unsigned char[file_info.uncompressed_size];
            // Entry is a file, so extract it.
            printf("file:%s\n", filename);
            //	fflush(stdout);
            if (unzOpenCurrentFile(_zipFile) != UNZ_OK) {
                delete[] buffer;
                printf("could not open file\n");
                return nullptr;
            }

            int error = UNZ_OK;

            error = unzReadCurrentFile(_zipFile, buffer, file_info.uncompressed_size);
            if (error < 0) {
                printf("error %d\n", error);
                unzCloseCurrentFile(_zipFile);
                delete[] buffer;
                return nullptr;
            }
            fileSize = file_info.uncompressed_size;
            return buffer;
        }

        unzCloseCurrentFile(_zipFile);

        // Go the the next entry listed in the zip file.
        if ((i + 1) < _globalInfo.number_entry) {
            if (unzGoToNextFile(_zipFile) != UNZ_OK) {
                printf("could not read next file\n");
                return nullptr;
            }
        }
    }
    return nullptr;
}