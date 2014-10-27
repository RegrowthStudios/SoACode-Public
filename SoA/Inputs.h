#pragma once

// These can not be an enum. They are very likely to change values at runtime!
extern i32 INPUT_BLOCK_SCANNER;
extern i32 INPUT_BLOCK_DRAG;
extern i32 INPUT_CROUCH;
extern i32 INPUT_DEBUG;
extern i32 INPUT_DRAW_MODE;
extern i32 INPUT_FLASH_LIGHT;
extern i32 INPUT_FLY;
extern i32 INPUT_GRID;
extern i32 INPUT_HORIZONTAL;
extern i32 INPUT_HUD;
extern i32 INPUT_INVENTORY;
extern i32 INPUT_JUMP;
extern i32 INPUT_MARKER;
extern i32 INPUT_MEGA_SPEED;
extern i32 INPUT_MOUSE_LEFT;
extern i32 INPUT_MOUSE_RIGHT;
extern i32 INPUT_PAUSE;
extern i32 INPUT_PHYSICS_BLOCK_UPDATES;
extern i32 INPUT_PLANET_DRAW_MODE;
extern i32 INPUT_PLANET_ROTATION;
extern i32 INPUT_RELOAD_BLOCKS;
extern i32 INPUT_RELOAD_SHADERS;
extern i32 INPUT_RELOAD_TEXTURES;
extern i32 INPUT_RELOAD_UI;
extern i32 INPUT_SONAR;
extern i32 INPUT_SCAN_WSO;
extern i32 INPUT_SPRINT;
extern i32 INPUT_UPDATE_FRUSTUM;
extern i32 INPUT_VERTICAL;
extern i32 INPUT_WATER_UPDATE;
extern i32 INPUT_ZOOM;
extern i32 INPUT_RANDOM_DEBUG;

// Initialize Input IDs At Runtime
extern void initInputs();