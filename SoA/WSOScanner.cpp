#include "stdafx.h"
#include "WSOScanner.h"

#include "WSO.h"
#include "WSOAtlas.h"
#include "WSOData.h"
#include "NChunkGrid.h"

// Scan A Radius Of (WSO_MAX_SIZE - 1) From The Center Block
const int WSO_QUERY_SIZE = WSO_MAX_SIZE * 2 - 1;

WSOScanner::WSOScanner(WSOAtlas* atlas) :
_wsoAtlas(atlas) {

}

bool checkForWSO(const i16* query, const WSOData* data, i32v3& offset) {
    i32v3 minPos(WSO_MAX_SIZE);
    minPos -= data->size;
    i32v3 localOff;

    // TODO: Check Y-Rotations

    // Loop Through Offsets
    for (offset.y = minPos.y; offset.y < WSO_MAX_SIZE; offset.y++) {
        for (offset.z = minPos.z; offset.z < WSO_MAX_SIZE; offset.z++) {
            for (offset.x = minPos.x; offset.x < WSO_MAX_SIZE; offset.x++) {

                // Try To Find A WSO At This Offset
                i32v3 maxPos(offset + data->size);
                bool isOK = true;
                i32 dataIndex = 0;
                for (localOff.y = offset.y; localOff.y < maxPos.y && isOK; localOff.y++) {
                    for (localOff.z = offset.z; localOff.z < maxPos.z && isOK; localOff.z++) {
                        for (localOff.x = offset.x; localOff.x < maxPos.x && isOK; localOff.x++) {
                            if (data->wsoIDs[dataIndex] != WSO_DONT_CARE_ID) {
                                i32 qIndex = (localOff.y * WSO_QUERY_SIZE + localOff.z) * WSO_QUERY_SIZE + localOff.x;
                                i16 id = query[qIndex];

                                if (data->wsoIDs[dataIndex] != id)
                                    isOK = false;
                                else
                                    printf("I Found One Charlie %d\n", dataIndex);
                            }
                            else
                                printf("I Don't Care Charlie %d\n", dataIndex);
                            dataIndex++;
                        }
                    }
                }

                // All Requirements Of WSO Are Met
                if (isOK) return true;
            }
        }
    }

    // Could Not Find A Single One
    return false;
}
std::vector<WSO*> WSOScanner::scanWSOs(const i32v3& position, NChunkGrid* cg) {
    //// Get A Piece Of The World
    //const i16* query = getQuery(position, cg);

    //std::vector<WSO*> wsos;

    //// Loop Through All Possible WSOs
    //i32v3 offset;
    //for (i32 i = _wsoAtlas->getSize() - 1; i >= 0; i--) {
    //    WSOData* data = _wsoAtlas->get(i);
    //    if (checkForWSO(query, data, offset)) {
    //        i32v3 localPos = offset - i32v3(WSO_MAX_SIZE - 1);
    //        localPos += position;

    //        // This Is A Possible WSO
    //        //TODO(Cristian) Make this work for new chunkmanager mapping
    //        //WSO* wso = new WSO(data, f64v3(localPos + cm->cornerPosition));
    //        //wsos.push_back(wso);
    //    }
    //}

    //// TODO: Make Sure We Don't Get Already Created WSOs

    //delete query;
    //return wsos;
    return std::vector<WSO*>();
}

const i16* WSOScanner::getQuery(const i32v3& position, NChunkGrid* cg) {
    //// Get The Query Based Off Of The Max Size Of The WSO
    //const i32v3 minPos = position - i32v3(WSO_MAX_SIZE - 1);
    //const i32v3 maxPos = position + i32v3(WSO_MAX_SIZE - 1);
    //return cg->getIDQuery(minPos, maxPos);
    return nullptr;
}
