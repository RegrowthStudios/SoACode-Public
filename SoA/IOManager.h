#pragma once

#include <boost/filesystem.hpp>

// Manages Simple IO Operations Within A File System
class IOManager {
public:
    // Create An IO Manager With Default Program Directories
    IOManager();
    ~IOManager();

    // Set Up IO Environment
    void setSearchDirectory(const cString s);
    static void setCurrentWorkingDirectory(const nString& s);
    static void setExecutableDirectory(const nString& s);

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

    // Return A String Which Is The Root Absolute Directory Of The Path
    static void getDirectory(const cString path, nString& resultPath);

    /// Gets all the entries in a directory
    /// @param dirPath: The directory to search
    /// @param entries: The store for all the resulting paths
    void getDirectoryEntries(nString dirPath, std::vector<boost::filesystem::path>& entries);

    // Attempt To Find An Absolute File Path (Must Be Deleted) With This Environment
    // Returns false on failure
    bool resolveFile(const cString path, nString& resultAbsolutePath);

    // Open A File Using STD Flags ("r", "w", "b", etc.) 
    // Returns NULL If It Can't Be Found
    FILE* openFile(const cString path, const cString flags);

    // Read An Entire File To A String
    // Returns false If File Can't Be Found
    bool readFileToString(const cString path, nString& data);
    bool readFileToData(const cString path, std::vector<ui8>& data);
private:
    // Search Order (Top-Down)
    cString _searchDir;
    static cString _cwDir;
    static cString _execDir;
};