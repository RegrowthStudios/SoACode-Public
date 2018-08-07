#include "stdafx.h"
#include "AmbienceLibrary.h"

void AmbienceLibrary::addTrack(const nString& list, const nString& name, const vpath& file) {
    m_lists[list][name] = file;
}

const AmbienceList& AmbienceLibrary::getTracks(const nString& name) const {
    return m_lists.at(name);
}
AmbienceLibrary::Track AmbienceLibrary::getTrack(const nString& list, Random& rGen) const {
    const AmbienceList& tracks = getTracks(list);

    ui32 trackNum = (ui32)(rGen.genMT() * tracks.size()) % tracks.size();
    printf("Chose track %d out of %zd in %s\n", trackNum + 1, tracks.size(), list.c_str());
    AmbienceList::const_iterator iter = tracks.cbegin();
    while (trackNum != 0) {
        iter++;
        trackNum--;
    }

    return Track(iter->first, iter->second);
}
