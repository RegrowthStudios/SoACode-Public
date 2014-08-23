#include "stdafx.h"
#include "FragFile.h"

// Fragmentation Seeking Information
struct FragBlockHeader {
public:
    // Path ID
    i32 id;
    // The Size Of The Current Block
    i32 sizeBlock;
    // Offset Into The File Relative To The End Of Data Block
    i32 offset;
};
// A Header For Each Data Path
struct FragHeader {
public:
    // The Total Size Of The Path
    i32 totalSize;
    // First Data Block
    FragBlockHeader firstBlock;
    // Last Data Block
    FragBlockHeader lastBlock;
};

FragFile::FragFile(i32 numPaths, const cString path, bool isReadonly) :
_numDataPaths(numPaths),
_headerSizeInBytes(_numDataPaths * sizeof(FragHeader)),
_file(nullptr),
_headers(nullptr),
_curPath(0) {
    bool isSuccess = isReadonly ? openReadonlyFile(path) : openWritingFile(path);
    if (!isSuccess) {
        _file = nullptr;
#ifdef DEBUG
        printf("Error Attempting To Open Fragmented File:\n%s\n", path);
#endif // DEBUG
    }
}
FragFile::~FragFile() {
    close();
}

void FragFile::setDataPath(i32 p) {
    if (p < 0 || p >= _numDataPaths) {
#ifdef DEBUG
        printf("Attempting To Access Invalid Path\n");
#endif // DEBUG
        exit(-1);
    }
    _curPath = p;
}

i32 FragFile::getDataPathSize() const {
    return _headers[_curPath].totalSize;
}

void FragFile::read(void* buf) {
    ui8* data = (ui8*)buf;

    // Get Data Block Information
    i32 bytesLeft = getDataPathSize();
    FragBlockHeader header = _headers[_curPath].firstBlock;

    // Move To The First Block
    fseek(_file, header.offset + _headerSizeInBytes, SEEK_SET);

    while (bytesLeft > 0) {
        // Read The Header For The Block
        fread(&header, sizeof(FragBlockHeader), 1, _file);

        // Read The Data Of The Current Block
        fread(data, 1, header.sizeBlock, _file);
        bytesLeft -= header.sizeBlock;

        // Move Pointers
        data += header.sizeBlock;

        // Seek The Next Block
        if (bytesLeft > 0) fseek(_file, header.offset, SEEK_CUR);
    }
}

void FragFile::append(void* data, i32 sizeInBytes) {
    // Move To The End Of The File
    fseek(_file, 0, SEEK_END);
    i32 initialSize = getDataPathSize();
    _headers[_curPath].totalSize += sizeInBytes;

    // Create The Header
    FragBlockHeader header = {};
    header.id = _curPath;
    header.sizeBlock = sizeInBytes;

    // Store Our Position
    i32 fpos = ftell(_file);

    // Write The Fragmented Data Block
    fwrite(&header, sizeof(FragBlockHeader), 1, _file);
    fwrite(data, 1, sizeInBytes, _file);

    // Allow Seek Time
    fseek(_file, _curPath * sizeof(FragHeader), SEEK_SET);

    // Exchange Info About Last Known Block
    i32 otherPos = _headers[_curPath].lastBlock.offset;
    _headers[_curPath].lastBlock.offset = fpos - _headerSizeInBytes;
    i32 otherSize = _headers[_curPath].lastBlock.sizeBlock;
    _headers[_curPath].lastBlock.sizeBlock = sizeInBytes;

    // Check If We've Written Anything Before
    if (initialSize == 0) {
        _headers[_curPath].firstBlock = _headers[_curPath].lastBlock;
    }

    // Write The New Header
    fwrite(&_headers[_curPath], sizeof(FragHeader), 1, _file);

    if (initialSize != 0) {
        // Go To The Previous Last Block
        fseek(_file, otherPos + _headerSizeInBytes, SEEK_SET);

        // Reset Offset Information
        header.offset = fpos - otherPos;
        header.offset -= otherSize + sizeof(FragBlockHeader);
        header.offset -= _headerSizeInBytes;

        // Overwrite
        header.sizeBlock = otherSize;
        fwrite(&header, sizeof(FragBlockHeader), 1, _file);
    }
}
void FragFile::overwrite(void* data, i32 fileDataOffset) {
    // TODO: Implement
}

void FragFile::defragment(const cString tmpFileName) {
    // TODO: Implement
}

void FragFile::flush() {
    fflush(_file);
}
void FragFile::close() {
    if (_file) {
        flush();
        fclose(_file);
        _file = nullptr;
    }
    if (_headers) {
        delete[] _headers;
        _headers = nullptr;
    }
}

bool FragFile::openReadonlyFile(const cString path) {
    // Check For A Path
    if (!path) return false;

    // Attempt To Open The File
    errno_t err = fopen_s(&_file, path, "rb");
    if (err != 0) return false;

    // Read The Headers (They Must Be There)
    fseek(_file, 0, SEEK_END);
    i32 len = ftell(_file);
    fseek(_file, 0, SEEK_SET);
    len -= ftell(_file);

    if (len < _headerSizeInBytes) {
        // The Headers Must Have Not Been Written
        fclose(_file);
        return false;
    } else {
        // Read The Headers
        _headers = new FragHeader[_numDataPaths];
        fread(_headers, sizeof(FragHeader), _numDataPaths, _file);
        return true;
    }
}
bool FragFile::openWritingFile(const cString path) {
    // Check For A Path
    if (!path) return false;

    // Attempt To Open The File
    errno_t err = fopen_s(&_file, path, "wb+");
    if (err != 0) return false;

    // Try To Read The Headers
    fseek(_file, 0, SEEK_END);
    i32 len = ftell(_file);
    fseek(_file, 0, SEEK_SET);
    len -= ftell(_file);

    if (len < _headerSizeInBytes) {
        // Write New Headers
        _headers = new FragHeader[_numDataPaths]();
        for (i32 i = 0; i < _numDataPaths; i++) {
            _headers[i].firstBlock.id = i;
            _headers[i].firstBlock.id = i;
        }
        fwrite(_headers, sizeof(FragHeader), _numDataPaths, _file);
    } else {
        // Read The Headers
        _headers = new FragHeader[_numDataPaths];
        fread(_headers, sizeof(FragHeader), _numDataPaths, _file);
    }
    return true;
}


