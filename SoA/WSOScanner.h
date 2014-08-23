#pragma once
class ChunkManager;
class WSO;
class WSOAtlas;

// A Scanner That Uses An Atlas Of Known WSOs To Attempt Find WSOs
class WSOScanner {
public:
    // A Scanner Begins Its Life Knowing About A Certain Atlas
    WSOScanner(WSOAtlas* atlas);

    // Retrieve All The WSOs
    std::vector<WSO*> scanWSOs(const i32v3& position, ChunkManager* cm);
private:
    // Obtain A Box Volume Of Voxel IDs
    const i16* getQuery(const i32v3& position, ChunkManager* cm);

    // This Does Not Have To Point To A Global Atlas Necessarily ;)
    WSOAtlas* _wsoAtlas;
};