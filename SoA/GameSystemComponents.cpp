#include "stdafx.h"
#include "GameSystemComponents.h"

KEG_TYPE_DEF(AabbCollidableComponent, AabbCollidableComponent, kt) {
    using namespace keg;
    kt.addValue("box", Value::basic(offsetof(AabbCollidableComponent, box), BasicType::F32_V3));
    kt.addValue("offset", Value::basic(offsetof(AabbCollidableComponent, offset), BasicType::F32_V3));
}
/*
//TODO(Ben): Properly implement this including the bitfields
KEG_TYPE_DEF(ParkourInputComponent, ParkourInputComponent, kt) {
    using namespace keg;
    kt.addValue("Acceleration", Value::basic(offsetof(ParkourInputComponent, acceleration), BasicType::F32));
    kt.addValue("MaxSpeed", Value::basic(offsetof(ParkourInputComponent, maxSpeed), BasicType::F32));
}

//TODO(BEN): Properly implement this including the bitfields
KEG_TYPE_DEF(FreeMoveInputComponent, FreeMoveInputComponent, kt) {
    using namespace keg;
    kt.addValue("Speed", Value::basic(offsetof(FreeMoveInputComponent, speed), BasicType::F32));
}*/

KEG_TYPE_DEF(SpacePositionComponent, SpacePositionComponent, kt) {
    using namespace keg;
    kt.addValue("position", Value::basic(offsetof(SpacePositionComponent, position), BasicType::F64_V3));
    kt.addValue("orientation", Value::basic(offsetof(SpacePositionComponent, orientation), BasicType::F64_V4));
}

KEG_TYPE_DEF(VoxelPositionComponent, VoxelPositionComponent, kt) {
    using namespace keg;
    kt.addValue("orientation", Value::basic(offsetof(VoxelPositionComponent, orientation), BasicType::F64_V4));
    //kt.addValue("GridPosition", Value::value(&VoxelPositionComponent::gridPosition));
    kt.addValue("voxelPosition", Value::basic(offsetof(VoxelPositionComponent, gridPosition), BasicType::F64_V3));
    kt.addValue("worldCubeFace", Value::basic(offsetof(VoxelPositionComponent, gridPosition) + offsetof(VoxelPosition3D, face), BasicType::ENUM));
}

KEG_TYPE_DEF(PhysicsComponent, PhysicsComponent, kt) {
    using namespace keg;
    kt.addValue("velocity", Value::basic(offsetof(PhysicsComponent, velocity), BasicType::F64_V3));
    kt.addValue("mass", Value::basic(offsetof(PhysicsComponent, mass), BasicType::F32));
}

KEG_TYPE_DEF(FrustumComponent, FrustumComponent, kt) {
    using namespace keg;
    kt.addValue("fov", Value::basic(0, BasicType::F32));
    kt.addValue("aspectRatio", Value::basic(sizeof(f32), BasicType::F32));
    kt.addValue("zNear", Value::basic(sizeof(f32) * 2, BasicType::F32));
    kt.addValue("zFar", Value::basic(sizeof(f32) *3, BasicType::F32));
}

KEG_TYPE_DEF(HeadComponent, HeadComponent, kt) {
    using namespace keg;
    kt.addValue("relOrientation", Value::basic(offsetof(HeadComponent, relativeOrientation), BasicType::F64_V4));
    kt.addValue("relPosition", Value::basic(offsetof(HeadComponent, relativePosition), BasicType::F64_V3));
    kt.addValue("neckLength", Value::basic(offsetof(HeadComponent, neckLength), BasicType::F64));
}

