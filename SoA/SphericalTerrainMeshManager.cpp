#include "stdafx.h"
#include "SphericalTerrainMeshManager.h"


SphericalTerrainMeshManager::SphericalTerrainMeshManager() {
}


void SphericalTerrainMeshManager::update() {
    #define NUM_REQUESTS 8U
    m_rpcManager.processRequests(NUM_REQUESTS);
}