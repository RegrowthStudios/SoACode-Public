///
/// MusicPlayer.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 11 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Will play music files from a playlist
/// TODO: Not finished yet
///

#pragma once

#ifndef MusicPlayer_h__
#define MusicPlayer_h__

#include <Vorb/VorbPreDecl.inl>
#include <Vorb/IO.h>

class SoaFileSystem;
DECL_VSOUND(class Engine)

class MusicPlayer {
public:
    void refreshLists(const SoaFileSystem& fs);

    void begin(vsound::Engine& engine);
    void stop();
private:
    void searchTree(const vdir& path);

    bool m_isRunning = false;

    std::vector<vpath> m_music;
    std::vector<vpath> m_playlists;
};

#endif // MusicPlayer_h__
