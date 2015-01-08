#pragma once
#include <Vorb/Vorb.h>

#include "VoxelPlanetMapper.h"
#include "Actor.h"
#include "Camera.h"
#include "Biome.h"
#include "Texture2d.h"
#include "global.h"

class Chunk;
class InputManager;

class CollisionData {
public:
    CollisionData() :
    xPush(0), yPush(0), zPush(0),
    xMove(0), yMove(0), zMove(0),
    headSquish(0), yDecel(0) {}

    void clear() {
        xPush = 1.0f;
        yPush = 1.0f;
        zPush = 1.0f;
        xMove = 0.0f;
        yMove = 0.0f;
        zMove = 0.0f;
        headSquish = 0.0f;
    }

    f32 xPush, yPush, zPush, xMove, yMove, zMove, headSquish;
    f32 yDecel;
};

class Player : public Actor {
public:
    Player();
    ~Player();

    void initialize(nString playerName, float aspectRatio);
    bool update(int worldRadius, InputManager* inputManager, bool isMouseIn, f64 Gravity, f64 AirFrictionForce);
    void setNearestPlanet(i32 WorldRadius, i32 EnterRadius, i32 PlanetRowSize);
    void checkFaceTransition();
    void crouchMovement(bool up);
    void removeItem(Item* item);
    void applyCollisionData();

    void flyToggle();

    void mouseMove(i32 relx, i32 rely);
    void zoom();

    //setters
    void setMoveSpeed(f32 accel, f32 max);
    void setMoveMod(f32 moveMod) {
        _moveMod = moveMod;
    }

    //getters
    const f32v3& chunkDirection() const {
        return _chunkCamera.getDirection();
    }
    const f32v3& chunkUp() const {
        return _chunkCamera.getUp();
    }

    const f32m4& chunkViewMatrix() const {
        return _chunkCamera.getViewMatrix();
    }
    const f32m4& chunkProjectionMatrix() const {
        return _chunkCamera.getProjectionMatrix();
    }

    const f32v3& worldDirection() const {
        return _worldCamera.getDirection();
    }
    const f32v3& worldUp() const {
        return _worldCamera.getUp();
    }
    const f32v3& worldRight() const {
        return _worldCamera.getRight();
    }

    const f32m4& worldViewMatrix() const {
        return _worldCamera.getViewMatrix();
    }
    const f32m4& worldProjectionMatrix() const {
        return _worldCamera.getProjectionMatrix();
    }

    Camera& getWorldCamera() {
        return _worldCamera;
    }
    Camera& getChunkCamera() {
        return _chunkCamera;
    }


    f32 getMoveMod() const {
        return _moveMod;
    }
    const nString& getName() const {
        return _name;
    }
    f32 getCrouch() const {
        return _crouch;
    }

    f64 vel;

    bool isFlying;
    bool isGrounded;
    bool isSprinting;
    bool isClimbing;
    bool isClinging;
    bool canCling;
    bool isSwimming;
    bool isUnderWater;
    bool isOnPlanet;

    Item* rightEquippedItem, *leftEquippedItem;
    i32 lightActive;

    i32 headInBlock, headVoxelLight, headSunLight;
    bool isDragBreak;
    Item* dragBlock;

    Biome* currBiome;
    i32 currTemp;
    i32 currHumidity;
    Chunk* currCh;

    volatile i32 scannedBlock;

    i32 planetRowSize;

    vvox::VoxelPlanetMapData voxelMapData;
    CollisionData collisionData;

    f32m4 worldRotationMatrix;

private:
    void moveInput(bool isMouseIn);
    void spaceMove();
    void planetMove();
    void groundMove();
    void flyModeMove();
    void updateCameras();
    void calculateWorldPosition();
    void calculateHeadPosition();

    nString _name;

    Camera _worldCamera, _chunkCamera;

    InputManager* m_inputManager = nullptr;

    f32 _acceleration, _maxVelocity;

    f32 _moveMod;
    f32 _mass;

    i32 _zoomLevel;
    f32 _zoomPercent;
    f32 _zoomSpeed;

    f32 _heightMod;
    f32 _stepMod;

    f32 _rolling;

    f32 _cameraBobTheta;

    f32 _crouch;

    bool _hasStartedRolling;

    bool _isJumping;
    i32 _jumpVal;
    i32 _maxJump;
    i32 _jumpWindup;
    f32 _jumpForce;
    f32 _gravity;
    f32 _airFrictionForce;
    f32 _friction;
    f32 _speed;

    i32 _worldRadius, _enterRadius;

    bool _wasSpacePressedLastFrame;
};
