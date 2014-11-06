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
    const cString readFileToString(const cString path);
    bool readFileToData(const cString path, std::vector<ui8>& data);

    /// Writes a string to a file. Creates file if it doesn't exist
    /// @param path: The path to the file
    /// @param data: The data to write to file
    /// @return true on success
    bool writeStringToFile(const cString path, const nString& data);

    /// Makes a directory
    /// @param path: The directory path to make
    bool makeDirectory(const cString path);

    /// Check if a file exists
    /// @param path: The path to the file
    /// @return true if file exists
    bool fileExists(const cString path);

    /// Check if a directory exists
    // @param path: The path to the directory
    /// @return true if directory exists
    bool directoryExists(const cString path);
private:
    // Search Order (Top-Down)
    cString _searchDir;
    static cString _cwDir;
    static cString _execDir;
};