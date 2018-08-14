#include "stdafx.h"
#include "MusicPlayer.h"

#include <Vorb/IO.h>
#include <Vorb/sound/SoundEngine.h>


#include "SoaFileSystem.h"

void MusicPlayer::refreshLists(const SoaFileSystem& fs) {
    // Kill current lists
    m_music.clear();
    m_playlists.clear();

    // Check for the music folder
    const vpath& musicPath = fs.get("Music").getSearchDirectory();
    if (!musicPath.isValid()) {
        // Create the folder and then exit
        vio::buildDirectoryTree(musicPath);
        return;
    }

    vdir musicDir;
    if (!musicPath.asDirectory(&musicDir)) {
        // What happened here...
        return;
    }
    searchTree(musicDir);
}

void MusicPlayer::begin(vsound::Engine& engine VORB_UNUSED) {
    if (m_isRunning) return;
    m_isRunning = true;
}
void MusicPlayer::stop() {
    if (!m_isRunning) return;
    m_isRunning = false;
}

inline bool strEndsWith(const nString& value, const nString& ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}
void MusicPlayer::searchTree(const vdir& root) {
    root.forEachEntry([&] (Sender s VORB_UNUSED, const vpath& p) {
        if (p.isFile()) {
            nString fileName = p.getLeaf();
            if (strEndsWith(fileName, ".mp3")) {
                m_music.push_back(p);
                printf("Found Music: %s\n", fileName.c_str());
            } else if (strEndsWith(fileName, ".ogg")) {
                m_music.push_back(p);
                printf("Found Music: %s\n", fileName.c_str());
            } else if (strEndsWith(fileName, ".playlist.yml")) {
                m_playlists.push_back(p);
                printf("Found Playlist: %s\n", fileName.c_str());
            } else {
                // Not a valid file
            }
        } else if (p.isDirectory()) {
            vdir child;
            p.asDirectory(&child);
            searchTree(child);
        }
    });
}
