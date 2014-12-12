#include "stdafx.h"
#include "Player.h"

#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "BlockData.h"
#include "GameManager.h"
#include "InputManager.h"
#include "Inputs.h"
#include "Item.h"
#include "Options.h"
#include "Rendering.h"
#include "Rendering.h"
#include "Texture2d.h"
#include "utils.h"

// TODO: Get Rid Of This
using namespace glm;

#define MAX(a,b) ((a)>(b)?(a):(b))

Player::Player() : scannedBlock(0),
                    _mass(100),
                    headInBlock(NONE),
                    rightEquippedItem(NULL),
                    leftEquippedItem(NULL),
                    lightActive(0),
                    isClimbing(0),
                    _cameraBobTheta(0),
                    _wasSpacePressedLastFrame(0),
                    isOnPlanet(1), 
                    _maxJump(40),
                    _crouch(0),
                    _jumpForce(0.16),
                    _jumpWindup(0),
                    _jumpVal(0),
                    _isJumping(0),
                    _heightMod(0.0f),
                    currBiome(NULL),
                    currTemp(-1),
                    currHumidity(-1),
                    _moveMod(1.0),
                    canCling(0),
                    isClinging(0),
                    isFlying(0),
                    _rolling(0),
                    _name("Nevin"),
                    isSprinting(0),
                    vel(0),
                    _speed(0.25f),
                    _zoomLevel(0),
                    _zoomSpeed(0.04f),
                    _zoomPercent(1.0f),
                    isGrounded(0),
                    _acceleration(0.02f),
                    _maxVelocity(0.21f),
                    currCh(NULL),
                    isSwimming(0),
                    isUnderWater(0),
                    _friction(0.006f),
                    isDragBreak(0),
                    dragBlock(NULL),
                    voxelMapData(0,0,0,0) {

    velocity.x = 0;
    velocity.y = 0;
    velocity.z = 0;
    

    facePosition.x = gridPosition.x = 0;
    facePosition.z = gridPosition.z = 0;

    // Initial position : on +Z

    // Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units

    velocity.y = 0.0f;

    _chunkCamera.setClippingPlane(0.2, 10000);

    boundingBox.x = 0.6f; //x radius
    boundingBox.y = 4.0f; //y total height
    boundingBox.z = 0.6f; //z radius

}

Player::~Player(void)
{
    for (size_t i = 0; i < inventory.size(); i++){
        delete inventory[i];
    }
}

void Player::initialize(nString playerName, float aspectRatio) {
    _name = playerName;
    _chunkCamera.init(aspectRatio);
    _worldCamera.init(aspectRatio);
}

void Player::updateCameras()
{

	_chunkCamera.update();
	_worldCamera.update();

    calculateHeadPosition();
    calculateWorldPosition();

	//rolling
	/*float rotTheta = _rolling * M_PI * 2;
	float rotTheta2 = sin(rotTheta/2) / 1.0f;
	
	vec3 rotRight = normalize(vec3(right.x, right.y + 0.3, right.z));
	quat rollQuat = quat(-cos(rotTheta/2), rotRight.x*sin(rotTheta/2), rotRight.y*sin(rotTheta/2), rotRight.z*sin(rotTheta/2));
	
	dmat4 qrot = dmat4(toMat4(rollQuat));
	
	direction = glm::normalize(glm::dvec3(qrot * glm::dvec4(direction, 1.0)));

	up = glm::normalize(glm::cross( right, direction ));
	right = glm::cross(direction, up);*/
}

bool Player::update(bool isMouseIn, double Gravity, double AirFrictionForce)
{
    _gravity = Gravity;
    _airFrictionForce = AirFrictionForce;

    updateCameras();

    if (canCling == 0 || isGrounded) isClinging = 0;

    moveInput(isMouseIn);
    
    isClimbing = 0;

    return 1;
}

void Player::moveInput(bool isMouseIn)
{
    if (isFlying) velocity = glm::dvec3(0,0,0);

    //cout << "X: " << position.x << "  Y: " << position.y << "  Z: " << position.z << endl;

    //if (glm::length(worldPosition) > GameManager::planet->radius*1.05){
    //    onPlanet = 0;
    //}
    //else{
        isOnPlanet = 1;
    //}

    if (!isFlying){
        if (isSwimming){
            velocity.y -= _gravity * glSpeedFactor * 0.05f * _moveMod;
        }else{ //probably should maybe move this to the integration?
            velocity.y -= _gravity * glSpeedFactor * _moveMod;
            if (isClinging && velocity.y < 0) velocity.y = 0; //so we dont slide down when clinging
        }
    }

    if (isMouseIn)
    {
        if (isOnPlanet){
            planetMove();
        }else{
            spaceMove();
        }
    }
    if (isSwimming){
        if (velocity.y < -0.08){
            velocity.y = MIN(0.1 + velocity.y, -0.08);
        }else if (velocity.y > 0.08){
            velocity.y = MAX(velocity.y - 0.1, 0.08);
        }
    }else{ //Terminal Velocity
        if (length(velocity) > 0){
            velocity -= _airFrictionForce*normalize(velocity) / _mass * glSpeedFactor;
        }
    }

    _chunkCamera.setPosition(headPosition);
    isSwimming = 0;
    isUnderWater = 0;
}

void Player::spaceMove()
{
    //int midX = 320;
    //int midY = 240;
    //int xpos, ypos;

    //if (Keys[SDLK_q].pr){
    //    _rollAngle -= 0.01;
    //}
    //else if (Keys[SDLK_e].pr){
    //    rollAngle += 0.01;
    //}
    //if (rollAngle < 0){
    //    rollAngle = 2 * M_PI;
    //}Fke
    //else if (rollAngle > 2 * M_PI){
    //    rollAngle = 0;
    //}

    //wasViewChange = 1;
    //worldDirection = glm::dvec3(
    //    cos(pitchAngle) * sin(yawAngle), 
    //    sin(pitchAngle),
    //    cos(pitchAngle) * cos(yawAngle)
    //);

    //// Right vector
    //worldRight = glm::dvec3(
    //    sin(yawAngle - 3.14159f/2.0f) * cos(rollAngle), 
    //    sin(rollAngle),
    //    cos(yawAngle - 3.14159f/2.0f) * cos(rollAngle)
    //);

    //worldUp = glm::cross(worldRight, worldDirection);

    //float altspeed = 20000; //350;
    //float ctrlSpeed = 50 + MAX((glm::length(worldPosition) - currentWorldRadius)*0.01*planetScale, 0);
    //if (Keys[SDLK_w].pr){
    //    worldPosition += worldDirection * glSpeedFactor * speed * (1.0 + Keys[SDLK_LCTRL].pr*ctrlSpeed + Keys[SDLK_LALT].pr*altspeed);
    //}else if (Keys[SDLK_s].pr){
    //    worldPosition -= worldDirection * glSpeedFactor * speed * (1.0 + Keys[SDLK_LCTRL].pr*ctrlSpeed + Keys[SDLK_LALT].pr*altspeed);
    //}
    //if (Keys[SDLK_d].pr){
    //    worldPosition += worldRight * glSpeedFactor * speed * (1.0 + Keys[SDLK_LCTRL].pr*ctrlSpeed + Keys[SDLK_LALT].pr*altspeed);
    //}else if (Keys[SDLK_a].pr){
    //    worldPosition -= worldRight * glSpeedFactor * speed * (1.0 + Keys[SDLK_LCTRL].pr*ctrlSpeed + Keys[SDLK_LALT].pr*altspeed);
    //}
}

void Player::planetMove()
{

    //do mouse move inout
    _hasStartedRolling = 0;
    if (collisionData.yDecel >= 0.35){
        _hasStartedRolling = 1;
        _rolling = 0.00001f;
    }

    if (_rolling != 0){ //rolling animation
        _rolling += 1 / 40.0f * glSpeedFactor;
        _crouch = _rolling*5;
        if (_crouch > 1.0) _crouch = 1.0;
        if (_rolling*5 > 4.0){
            _crouch = 5 - _rolling*4.5;
        }
        if (_rolling >= 1.0f) _rolling = 0.0f;
    }
        
    if (isFlying){
        flyModeMove();
    }else{
        groundMove();
    }

    //calculate the zoom
    if (_zoomLevel == 0){
        if (_zoomPercent > 1.0){
            _zoomPercent -= (float)((_zoomSpeed+_zoomSpeed)*glSpeedFactor);
        }else{
            _zoomPercent = 1.0;
        }
    }else if (_zoomLevel == 1){
        if (_zoomPercent < 2.0){
            _zoomPercent += (float)(_zoomSpeed*glSpeedFactor);
        }else{
            _zoomPercent = 2.0;
        }
    }else if (_zoomLevel == 2){
        if (_zoomPercent < 4.0){
            _zoomPercent += (float)((_zoomSpeed + _zoomSpeed)*glSpeedFactor);
        }else{
            _zoomPercent = 4.0;
        }
    }

    //camera bob
    vel = sqrt(velocity.x*velocity.x + velocity.z*velocity.z);
    if (vel > 0.075){
        if (isGrounded) _cameraBobTheta += vel*glSpeedFactor*0.75;
    }else if(_cameraBobTheta && _cameraBobTheta != M_PI){
        if (_cameraBobTheta > M_PI && _cameraBobTheta < M_PI*1.5){
            _cameraBobTheta -= 0.05*glSpeedFactor;
            if (_cameraBobTheta <= M_PI){
                _cameraBobTheta = M_PI;
            }
        }
        else if (_cameraBobTheta < M_PI && _cameraBobTheta > M_PI * 0.5){
            _cameraBobTheta += 0.05*glSpeedFactor;
            if (_cameraBobTheta >= M_PI){
                _cameraBobTheta = M_PI;
            }
        }
        else if (_cameraBobTheta < M_PI * 0.5){
            _cameraBobTheta -= 0.05f*glSpeedFactor;
            if (_cameraBobTheta < 0){
                _cameraBobTheta = 0;
            }
        }
        else if (_cameraBobTheta > M_PI*1.5){
            _cameraBobTheta += 0.05*glSpeedFactor;
            if (_cameraBobTheta >= 2 * M_PI){
                _cameraBobTheta = 0;
            }
        }
    }
    if (_cameraBobTheta > 2 * M_PI) _cameraBobTheta -= 2 * M_PI;

    //if (glm::length(worldPosition) > enterRadius) onPlanet = 0;
}

void Player::mouseMove(int relx, int rely)
{
    if (gameOptions.invertMouse) rely *= -1;
    // Compute new orientation
    
    float senstivityMult = (gameOptions.mouseSensitivity / 100.0f * 2.75 + 0.25);
    const float mouseSpeed = 0.1;

    //_chunkCamera.offsetAngles(-mouseSpeed / _zoomPercent* double(-rely) * senstivityMult, -mouseSpeed / _zoomPercent* double(-relx) * senstivityMult);
    updateCameras();
}

void Player::groundMove()
{
    double maxVel = _maxVelocity;

    const glm::vec3 &direction = _chunkCamera.getDirection();
    const glm::vec3 &right = _chunkCamera.getRight();
    const glm::vec3 &up = _chunkCamera.getUp();

    vec3 walkDir = vec3(0.0f);
    vec3 planeDir;
    
    if (isSwimming){
        planeDir = glm::normalize(direction);
    }else{
        planeDir = glm::normalize(glm::vec3(direction.x, 0.0f, direction.z));    
    }
    vec3 velChangeRay;
    float dirAgility;
    
    float forwardInput = GameManager::inputManager->getAxis(INPUT_VERTICAL);
    float rightInput = GameManager::inputManager->getAxis(INPUT_HORIZONTAL);

    walkDir = planeDir * forwardInput;
    walkDir += right * rightInput;
    if (std::abs(rightInput) != 0) {
        walkDir = glm::normalize(walkDir);
    }

    if (_moveMod < 1.0){ //we want mud to have fast decel but slow acel
        if (_moveMod == 0){
            dirAgility = 0;
        }
        else if (walkDir.x == 0 && walkDir.z == 0){ //we are decelerating
            dirAgility = _acceleration * glSpeedFactor * (1.0 + (1.0 - _moveMod));
        }
        else{
            dirAgility = _acceleration * glSpeedFactor * _moveMod; //looks like this always happens.
        }
    }
    else{ //we want ice to have slow accel and decel
        if (direction.x == 0 && direction.z == 0){ //we are decelerating
            dirAgility = _acceleration * glSpeedFactor * (1.0 / _moveMod);
        }
        else{
            dirAgility = _acceleration * glSpeedFactor * (1.0 / _moveMod);
        }
    }

    if (GameManager::inputManager->getKey(INPUT_SPRINT)){
        isSprinting = true;
        maxVel *= 1.5;
    }else{
        isSprinting = false;
    }
    bool inputJumping = GameManager::inputManager->getKey(INPUT_JUMP);
    bool inputCrouching = GameManager::inputManager->getKey(INPUT_CROUCH);
    //********************* crouch
    if (inputCrouching){
        crouchMovement(0);
    }else if (_crouch){
        crouchMovement(1);
    }
    if (!_crouch){
        if (isGrounded){
            if (inputJumping && !_isJumping){
                if (!_wasSpacePressedLastFrame){
                    if (_moveMod < 1.0){
                        velocity.y = _jumpForce*_moveMod;
                    }else{
                        velocity.y = _jumpForce;
                    }
                }
            }
        }else if (!isSwimming){
            if (inputJumping && !_wasSpacePressedLastFrame && isClinging && !isClimbing){ //jumping from a cling position
                velocity.y += _jumpForce/4.0f;
                velocity += _jumpForce * direction;
            }
        }else{ //swimming
            if (inputJumping){
                walkDir += vec3(0.0f, 1.0f, 0.0f);
                walkDir = normalize(walkDir);
            }
        }
    }

    _wasSpacePressedLastFrame = inputJumping; //so we know when a jump is initiated. No holding down spacebar!

    if (_crouch) maxVel *= (1.0f - 0.75f*_crouch);

    if (isSwimming){
        maxVel *= 0.5;
        dirAgility *= 0.3;
    }else if (!isGrounded && !isClimbing){
        dirAgility *= 0.1; //agility stat could help here
    }

    if (_moveMod < 1.0){
        walkDir *= maxVel * _moveMod; //multiply walk dir by speed to get it to be the final velocity
        //if walk direction is not zero, first we move the velocity vector towards the walk dir vector
    }
    else{
        walkDir *= maxVel;
    }
    if (isSwimming){
        velChangeRay = walkDir - vec3(velocity);
    }else{
        velChangeRay = walkDir - vec3(velocity.x, 0.0, velocity.z);
    }

    if (velChangeRay.length()) velChangeRay = normalize(velChangeRay);
    if (velocity.x < walkDir.x){
        velocity.x += velChangeRay.x*dirAgility; // move the velocity vector towards the new velocity vector using agility
        if (velocity.x > walkDir.x) velocity.x = walkDir.x; //if we pushed it too far pull it back
    }else if (velocity.x > walkDir.x){
        velocity.x += velChangeRay.x*dirAgility;
        if (velocity.x < walkDir.x) velocity.x = walkDir.x;
    }

    if (velocity.z < walkDir.z){
        velocity.z += velChangeRay.z*dirAgility;
        if (velocity.z > walkDir.z) velocity.z = walkDir.z;
    }else if (velocity.z > walkDir.z){
        velocity.z += velChangeRay.z*dirAgility;
        if (velocity.z < walkDir.z) velocity.z = walkDir.z;
    }

    if (isSwimming){
        if (velocity.y < walkDir.y){
            velocity.y += velChangeRay.y*dirAgility;
            if (velocity.y > walkDir.y) velocity.y = walkDir.y;
        }else if (velocity.y > walkDir.y){
            velocity.y += velChangeRay.y*dirAgility;
            if (velocity.y < walkDir.y) velocity.y = walkDir.y;
        }
    }
    
    if (_hasStartedRolling){
        float velLength = length(vec3(velocity.x, 0, velocity.z));
        float pushOff = 0.35 * collisionData.yDecel;
        vec3 dir = normalize(vec3(velocity.x, 0, velocity.z));
        if (velLength != 0){
            if (velLength < pushOff){
                velocity.x += dir.x * (pushOff-velLength);
                velocity.z += dir.z * (pushOff-velLength);
            }
        }else{
            velocity.x += planeDir.x * (pushOff-velLength);
            velocity.z += planeDir.z * (pushOff-velLength);
        }
    }
}

void Player::flyModeMove()
{
    float forwardInput = GameManager::inputManager->getAxis(INPUT_VERTICAL);
    float rightInput = GameManager::inputManager->getAxis(INPUT_HORIZONTAL);
    float jumpInput = GameManager::inputManager->getAxis(INPUT_JUMP);
    float sprintInput = GameManager::inputManager->getAxis(INPUT_SPRINT);
    float crouchInput = GameManager::inputManager->getAxis(INPUT_CROUCH);
    float megaSpeedInput = GameManager::inputManager->getAxis(INPUT_MEGA_SPEED);

    vel = 0;
    if (jumpInput > 0){
        gridPosition.y += _maxVelocity;
        facePosition.y += _maxVelocity;
    } else if (sprintInput > 0){
        gridPosition.y -= _maxVelocity;
        facePosition.y -= _maxVelocity;
    }

    const glm::vec3 direction = _chunkCamera.getDirection();
    const glm::vec3 right = _chunkCamera.getRight();

    float altspeed = 20000; //350;
    float ctrlSpeed = 50 + MAX((glm::length(worldPosition) - _worldRadius)*0.02*planetScale, 0);
    
    gridPosition += forwardInput * direction * glSpeedFactor * _speed * (1.0f + crouchInput*ctrlSpeed + megaSpeedInput*altspeed);
    facePosition += forwardInput * direction * glSpeedFactor * _speed * (1.0f + crouchInput*ctrlSpeed + megaSpeedInput*altspeed);

    gridPosition += rightInput * right * glSpeedFactor * _speed * (1.0f + crouchInput*ctrlSpeed + megaSpeedInput*altspeed);
    facePosition += rightInput * right * glSpeedFactor * _speed * (1.0f + crouchInput*ctrlSpeed + megaSpeedInput*altspeed);

    checkFaceTransition();
}

void Player::setNearestPlanet(int WorldRadius, int EnterRadius, int PlanetRowSize)
{
    _worldRadius = WorldRadius;
    _enterRadius = EnterRadius;
    planetRowSize = PlanetRowSize;
}

void Player::checkFaceTransition()
{
    int i;
    int newFace;
    if (facePosition.x/32 > planetRowSize*0.5){
        facePosition.x -= planetRowSize*32;
        i = voxelMapData.rotation;
        newFace = vvox::FaceNeighbors[voxelMapData.face][i];
        voxelMapData.rotation += vvox::FaceTransitions[voxelMapData.face][newFace];
        if (voxelMapData.rotation < 0){ voxelMapData.rotation += 4; }else{ voxelMapData.rotation %= 4; }
        voxelMapData.face = newFace;
    }else if (facePosition.x/32 < -planetRowSize*0.5){
        facePosition.x += planetRowSize*32;
        i = (2 + voxelMapData.rotation)%4;
        newFace = vvox::FaceNeighbors[voxelMapData.face][i];
        voxelMapData.rotation += vvox::FaceTransitions[voxelMapData.face][newFace];
        if (voxelMapData.rotation < 0){ voxelMapData.rotation += 4; }else{ voxelMapData.rotation %= 4; }
        voxelMapData.face = newFace;
    }
    if (facePosition.z/32 > planetRowSize*0.5){
        facePosition.z -= planetRowSize*32;
        i = (3 + voxelMapData.rotation)%4;
        newFace = vvox::FaceNeighbors[voxelMapData.face][i];
        voxelMapData.rotation += vvox::FaceTransitions[voxelMapData.face][newFace];
        if (voxelMapData.rotation < 0){ voxelMapData.rotation += 4; }else{ voxelMapData.rotation %= 4; }
        voxelMapData.face = newFace;
    }else if (facePosition.z/32 < -planetRowSize*0.5){
        facePosition.z += planetRowSize*32;
        i = (1 + voxelMapData.rotation)%4;
        newFace = vvox::FaceNeighbors[voxelMapData.face][i];
        voxelMapData.rotation += vvox::FaceTransitions[voxelMapData.face][newFace];
        if (voxelMapData.rotation < 0){ voxelMapData.rotation += 4; }else{ voxelMapData.rotation %= 4; }
        voxelMapData.face = newFace;
    }
}

void Player::calculateWorldPosition()
{
    int ipos, rpos, jpos;
    double incI, incJ, magnitude;
    glm::dvec3 v1, v2, v3;
    glm::vec3 tangent, biTangent;

    //Imagine the cube unfolded. Each face is like the following image for calculating tangent and bitangent
    // _____________
    // |           |
    // |        j  |
    // |       --->|
    // |     |i    |
    // |_____V_____|

    ipos = vvox::FaceCoords[voxelMapData.face][voxelMapData.rotation][0];
    jpos = vvox::FaceCoords[voxelMapData.face][voxelMapData.rotation][1];
    rpos = vvox::FaceCoords[voxelMapData.face][voxelMapData.rotation][2];
    incI = vvox::FaceSigns[voxelMapData.face][voxelMapData.rotation][0];
    incJ = vvox::FaceSigns[voxelMapData.face][voxelMapData.rotation][1];
    v1[ipos] = incI * facePosition.z;
    v1[jpos] = incJ * facePosition.x;
    v1[rpos] = vvox::FaceRadialSign[voxelMapData.face] * _worldRadius * planetScale;

    worldPosition = (headPosition.y*invPlanetScale + (double)_worldRadius)*glm::normalize(v1);
    incI *= 100;
    incJ *= 100;

    v1 = worldPosition;
    //need v2 and v3 for computing tangent and bitangent
    v2[ipos] = (double)worldPosition[ipos];
    v2[jpos] = (double)worldPosition[jpos]+incJ;
    v2[rpos] = worldPosition[rpos];

    v3[ipos] = (double)worldPosition[ipos]+incI;
    v3[jpos] = (double)worldPosition[jpos];
    v3[rpos] = worldPosition[rpos];

    //normalize them all
    magnitude = sqrt(v1.x*v1.x+v1.y*v1.y+v1.z*v1.z);
    v1 /= magnitude;
    magnitude = sqrt(v2.x*v2.x+v2.y*v2.y+v2.z*v2.z);
    v2 /= magnitude;
    magnitude = sqrt(v3.x*v3.x+v3.y*v3.y+v3.z*v3.z);
    v3 /= magnitude;

    tangent = glm::vec3(glm::normalize(v2 - v1));
    biTangent = glm::vec3(glm::normalize(v3 - v1));

    worldRotationMatrix[0] = glm::vec4(tangent, 0);
    worldRotationMatrix[1] = glm::vec4(v1, 0);
    worldRotationMatrix[2] = glm::vec4(biTangent, 0);
    worldRotationMatrix[3] = glm::vec4(0, 0, 0, 1);

    const glm::vec3 direction = _chunkCamera.getDirection();
    const glm::vec3 right = _chunkCamera.getRight();

    glm::vec3 worldDirection = glm::normalize( glm::vec3 (worldRotationMatrix * glm::vec4(direction, 1)));

    glm::vec3 worldRight = glm::normalize(glm::vec3(worldRotationMatrix * glm::vec4(right, 1)));

    glm::vec3 worldUp = glm::normalize(glm::cross( worldRight, worldDirection ));
    
    worldRight = glm::normalize(glm::cross(worldDirection, worldUp));

    _worldCamera.setPosition(worldPosition);
    _worldCamera.setDirection(worldDirection);
    _worldCamera.setRight(worldRight);
    _worldCamera.setUp(worldUp);

    //cout << glm::dot(worldRight, worldDirection) << endl;

    //worldRight = glm::normalize(glm::cross(worldUp, worldDirection));
}

void Player::calculateHeadPosition()
{
    double bobSinTheta = sin(_cameraBobTheta);
    double bobCosTheta = cos(_cameraBobTheta);

    const glm::vec3 &right = _chunkCamera.getRight();

    headPosition = gridPosition;
    headPosition.x += right.x*(bobCosTheta/7.0);
    headPosition.z += right.z*(bobCosTheta/7.0);
    headPosition.y += 3.55 - _heightMod + ABS(bobSinTheta/4.0);

    _chunkCamera.setPosition(headPosition);
}

void Player::flyToggle()
{
    isFlying = !isFlying;
    velocity.y = 0.0f;
}

void Player::setMoveSpeed(float accel, float max)
{
    _acceleration = accel;
    _maxVelocity = max;
}

void Player::crouchMovement(bool up){

    float crouchChangeSpeed = 0.1f;
    if (up){
        _crouch -= crouchChangeSpeed;
        if (_crouch < 0) _crouch = 0;
    }else{
        _crouch += crouchChangeSpeed;
        if (_crouch > 1.0f) _crouch = 1.0f;
    }
    _heightMod = _crouch*2.0f;
    boundingBox.y = 4.0 - _heightMod;
}

void Player::removeItem(Item *item)
{
    for (size_t i = 0; i < inventory.size(); i++){
        if (inventory[i] == item){
            inventory[i] = inventory.back();
            inventory.pop_back();
            break;
        }
    }
    delete item;
}

void Player::applyCollisionData()
{
    if (ABS(collisionData.xMove) > 10 || ABS(collisionData.yMove) > 10 || ABS(collisionData.zMove > 10) ||
        ABS(collisionData.xPush) > 1.0 || ABS(collisionData.yPush) > 1.0 || ABS(collisionData.zPush) > 1.0){
        std::cout << "HUGE COLLISION BUG: " << collisionData.xMove << " " << collisionData.yMove << " " << collisionData.zMove << " " << collisionData.xPush << " " << collisionData.yPush << " " << collisionData.zPush << std::endl;
        pError("ApplyCollisionData Error!");
    }

    gridPosition.x += collisionData.xMove;
    facePosition.x += collisionData.xMove;
    gridPosition.y += collisionData.yMove;
    facePosition.y += collisionData.yMove;
    gridPosition.z += collisionData.zMove;
    facePosition.z += collisionData.zMove;

    velocity.x *= collisionData.xPush;
    velocity.y *= collisionData.yPush;
    velocity.z *= collisionData.zPush;
    collisionData.headSquish += collisionData.yMove; //additional squish caused by y movement! (optional?)

    _crouch += collisionData.headSquish/2.0; // /2.0 puts it in the correct ratio since a full crouch is 2 blocks
    if (_crouch > 1.0f) _crouch = 1.0f;
    
    _heightMod = _crouch*2.0f;
    boundingBox.y = 4.0 - _heightMod;
}

void Player::zoom()
{
    _zoomLevel++;
    if (_zoomLevel == 3){
        _zoomLevel = 0;    
    }
}

