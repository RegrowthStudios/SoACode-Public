#include "stdafx.h"

#include "App.h"
#include "Camera.h"
#include "GLProgramManager.h"
#include "GameSystem.h"
#include "PlanetLoader.h"
#include "PlanetLoader.h"
#include "SoaState.h"
#include "SpaceSystem.h"
#include "SphericalTerrainGenerator.h"
#include "SphericalTerrainMeshManager.h"
#include "SpaceSystemFactories.h"

#include <Vorb/io/IOManager.h>
#include <Vorb/io/Keg.h>
#include <Vorb/TextureRecycler.hpp>
#include <Vorb/utils.h>
#include <yaml-cpp/yaml.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>


#define M_PER_KM 1000.0

SpaceSystem::SpaceSystem() : vcore::ECS() {
    // Add in component tables
    addComponentTable(SPACE_SYSTEM_CT_NAMEPOSITIION_NAME, &m_namePositionCT);
    addComponentTable(SPACE_SYSTEM_CT_AXISROTATION_NAME, &m_axisRotationCT);
    addComponentTable(SPACE_SYSTEM_CT_ORBIT_NAME, &m_orbitCT);
    addComponentTable(SPACE_SYSTEM_CT_SPHERICALTERRAIN_NAME, &m_sphericalTerrainCT);
    addComponentTable(SPACE_SYSTEM_CT_SPHERICALGRAVITY_NAME, &m_sphericalGravityCT);
    addComponentTable(SPACE_SYSTEM_CT_SPHERICALVOXEL_NAME, &m_sphericalVoxelCT);

    m_planetLoader = new PlanetLoader(&m_ioManager);
   
    #define MAX_NORMAL_MAPS 512U
    m_normalMapRecycler = new vg::TextureRecycler((ui32)PATCH_NORMALMAP_WIDTH,
                                                  (ui32)PATCH_NORMALMAP_WIDTH,
                                                  &SamplerState::POINT_CLAMP,
                                                  0,
                                                  vg::TextureInternalFormat::RGB8,
                                                  MAX_NORMAL_MAPS);
}

SpaceSystem::~SpaceSystem() {
    // Empty
}

void SpaceSystem::targetBody(const nString& name) {
    auto& it = m_bodyLookupMap.find(name);
    if (it != m_bodyLookupMap.end()) {
        m_targetEntity = it->second;
        m_targetComponent = m_namePositionCT.getComponentID(m_targetEntity);
    }
}

void SpaceSystem::targetBody(vcore::EntityID eid) {
    m_targetEntity = eid;
    m_targetComponent = m_namePositionCT.getComponentID(m_targetEntity);
}

void SpaceSystem::nextTarget() {
    auto& it = m_bodyLookupMap.find(m_namePositionCT.get(m_targetComponent).name);
    it++;
    if (it == m_bodyLookupMap.end()) it = m_bodyLookupMap.begin();
    m_targetEntity = it->second;
    m_targetComponent = m_namePositionCT.getComponentID(m_targetEntity);
}

void SpaceSystem::previousTarget() {
    auto& it = m_bodyLookupMap.find(m_namePositionCT.get(m_targetComponent).name);
    if (it == m_bodyLookupMap.begin()) it = m_bodyLookupMap.end();
    it--;
    m_targetEntity = it->second;
    m_targetComponent = m_namePositionCT.getComponentID(m_targetEntity);
}

f64v3 SpaceSystem::getTargetPosition() {
    m_mutex.lock();
    f64v3 pos = m_namePositionCT.get(m_targetComponent).position;
    m_mutex.unlock();
    return pos;
}

void SpaceSystem::addPlanet(const SystemBodyKegProperties* sysProps, const PlanetKegProperties* properties, SystemBody* body) {

  
}

void SpaceSystem::addStar(const SystemBodyKegProperties* sysProps, const StarKegProperties* properties, SystemBody* body) {
    body->entity = new vcore::Entity(addEntity());
    const vcore::EntityID& id = body->entity->id;

    vcore::ComponentID npCmp = addComponent(SPACE_SYSTEM_CT_NAMEPOSITIION_NAME, id);
    vcore::ComponentID arCmp = addComponent(SPACE_SYSTEM_CT_AXISROTATION_NAME, id);
    vcore::ComponentID oCmp = addComponent(SPACE_SYSTEM_CT_ORBIT_NAME, id);
    vcore::ComponentID sgCmp = addComponent(SPACE_SYSTEM_CT_SPHERICALGRAVITY_NAME, id);

    f64v3 up(0.0, 1.0, 0.0);
    m_axisRotationCT.get(arCmp).init(properties->angularSpeed,
                                     0.0,
                                     quatBetweenVectors(up, glm::normalize(properties->axis)));

    m_sphericalGravityCT.get(sgCmp).init(properties->diameter / 2.0,
                                         properties->mass);

    // Set the name
    m_namePositionCT.get(npCmp).name = body->name;

    setOrbitProperties(oCmp, sysProps);
}

void SpaceSystem::addGasGiant(const SystemBodyKegProperties* sysProps, const GasGiantKegProperties* properties, SystemBody* body) {
    body->entity = new vcore::Entity(addEntity());
    const vcore::EntityID& id = body->entity->id;

    vcore::ComponentID npCmp = addComponent(SPACE_SYSTEM_CT_NAMEPOSITIION_NAME, id);
    vcore::ComponentID arCmp = addComponent(SPACE_SYSTEM_CT_AXISROTATION_NAME, id);
    vcore::ComponentID oCmp = addComponent(SPACE_SYSTEM_CT_ORBIT_NAME, id);
    vcore::ComponentID sgCmp = addComponent(SPACE_SYSTEM_CT_SPHERICALGRAVITY_NAME, id);

    f64v3 up(0.0, 1.0, 0.0);
    m_axisRotationCT.get(arCmp).init(properties->angularSpeed,
                                     0.0,
                                     quatBetweenVectors(up, glm::normalize(properties->axis)));

    m_sphericalGravityCT.get(sgCmp).init(properties->diameter / 2.0,
                                         properties->mass);

    // Set the name
    m_namePositionCT.get(npCmp).name = body->name;

    setOrbitProperties(oCmp, sysProps);
}

void SpaceSystem::setOrbitProperties(vcore::ComponentID cmp, const SystemBodyKegProperties* sysProps) {
    // Set basic properties
    auto& orbitCmp = m_orbitCT.get(cmp);
    orbitCmp.eccentricity = sysProps->eccentricity;
    orbitCmp.orbitalPeriod = sysProps->period;
    orbitCmp.pathColor = sysProps->pathColor;

    // Calculate orbit orientation from normal
    const f64v3 right(1.0, 0.0, 0.0);
    if (right == -sysProps->orbitNormal) {
        f64v3 eulers(0.0, M_PI, 0.0);
        orbitCmp.orientation = f64q(eulers);
    } else if (right != sysProps->orbitNormal) {
        orbitCmp.orientation = quatBetweenVectors(right, sysProps->orbitNormal);
    }
}
