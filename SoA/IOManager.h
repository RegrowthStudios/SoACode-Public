#pragma once

// Manages Simple IO Operations Within A File System
class IOManager {
public:
    // Create An IO Manager With Default Program Directories
    IOManager();
    ~IOManager();

    // Set Up IO Environment
    void setSearchDirectory(const cString s);
    static void setCurrentWorkingDirectory(const cString s);
    static void setExecutableDirectory(const cString s);

    // Get The Current Environment State
    const cString getSearchDirectory() const {
        return _searchDir;
    }
    static const cString getCurrentWorkingDirectory()  {
        return _cwDir;
    }
    static const cString getExecutableDirectory()  {
        return _execDir;
    }

    // Return A String (Must Be Deleted) Which Is The Root Absolute Directory Of The Path
    static const cString getDirectory(const cString path);
    // Attempt To Find An Absolute File Path (Must Be Deleted) With This Environment
    const cString resolveFile(const cString path);

    // Open A File Using STD Flags ("r", "w", "b", etc.) 
    // Returns NULL If It Can't Be Found
    FILE* openFile(const cString path, const cString flags);

    // Read An Entire File To A String
    // Returns NULL If File Can't Be Found
    const cString readFileToString(const cString path);
    const cString readFileToData(const cString path, i32* len);
private:
    // Search Order (Top-Down)
    cString _searchDir;
    static cString _cwDir;
    static cString _execDir;
};