#pragma once
// Header For Each Data Path That Allows Fragmented Functionality
class FragHeader;

// A File That Stores Multiple Data Paths In Fragmented Blocks
class FragFile {
public:
    FragFile(i32 numPaths, const cString path, bool isReadonly);
    ~FragFile();

    // Point To A Data Path
    void setDataPath(i32 p);

    // Total Number Of Data Paths Found In The File
    const i32 getNumDataPaths() const {
        return _numDataPaths;
    }

    // Size In Bytes Of The Data Path
    i32 getDataPathSize() const;

    // Reads An Entire Data Path Into A Buffer
    void read(void* buf);

    // Appends Data Into The Current Data Path
    void append(void* data, i32 sizeInBytes);
    // Overwrites/Appends Data From An Offset Into The Current Data Path
    void overwrite(void* data, i32 fileDataOffset);

    // Defragment The File By Using A Temporary File Name (Will Perform A Swap/Delete On The Temp File)
    void defragment(const cString tmpFileName);

    // OS File Operations
    void flush();
    void close();
private:
    // IO Helpers
    bool openReadonlyFile(const cString path);
    bool openWritingFile(const cString path);

    // The Actual File
    FILE* _file;

    // Which Data Path To Read From
    i32 _curPath;

    // Data Path Information
    const i32 _numDataPaths;
    const i32 _headerSizeInBytes;
    FragHeader* _headers;
};