///
/// Inputs.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 29 Dec 2014
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Hard-coded list of inputs
/// TODO: Remove
///

#pragma once

#ifndef Inputs_h__
#define Inputs_h__

#include "InputMapper.h"

// These can not be an enum. They are very likely to change values at runtime!
extern InputMapper::InputID INPUT_BACKWARD;
extern InputMapper::InputID INPUT_BLOCK_DRAG;
extern InputMapper::InputID INPUT_BLOCK_SCANNER;
extern InputMapper::InputID INPUT_CROUCH;
extern InputMapper::InputID INPUT_CYCLE_COLOR_FILTER;
extern InputMapper::InputID INPUT_DEBUG;
extern InputMapper::InputID INPUT_DEV_CONSOLE;
extern InputMapper::InputID INPUT_DRAW_MODE;
extern InputMapper::InputID INPUT_EXIT;
extern InputMapper::InputID INPUT_FLASH_LIGHT;
extern InputMapper::InputID INPUT_FLY;
extern InputMapper::InputID INPUT_FORWARD;
extern InputMapper::InputID INPUT_GRID;
extern InputMapper::InputID INPUT_HUD;
extern InputMapper::InputID INPUT_INVENTORY;
extern InputMapper::InputID INPUT_JUMP;
extern InputMapper::InputID INPUT_LEFT;
extern InputMapper::InputID INPUT_LEFT_ROLL;
extern InputMapper::InputID INPUT_MARKER;
extern InputMapper::InputID INPUT_MEGA_SPEED;
extern InputMapper::InputID INPUT_MOUSE_LEFT;
extern InputMapper::InputID INPUT_MOUSE_RIGHT;
extern InputMapper::InputID INPUT_NIGHT_VISION;
extern InputMapper::InputID INPUT_NIGHT_VISION_RELOAD;
extern InputMapper::InputID INPUT_PAUSE;
extern InputMapper::InputID INPUT_PHYSICS_BLOCK_UPDATES;
extern InputMapper::InputID INPUT_PLANET_DRAW_MODE;
extern InputMapper::InputID INPUT_PLANET_ROTATION;
extern InputMapper::InputID INPUT_RANDOM_DEBUG;
extern InputMapper::InputID INPUT_RELOAD_BLOCKS;
extern InputMapper::InputID INPUT_RELOAD_SHADERS;
extern InputMapper::InputID INPUT_RELOAD_SYSTEM;
extern InputMapper::InputID INPUT_RELOAD_TARGET;
extern InputMapper::InputID INPUT_RELOAD_TEXTURES;
extern InputMapper::InputID INPUT_RELOAD_UI;
extern InputMapper::InputID INPUT_RIGHT;
extern InputMapper::InputID INPUT_RIGHT_ROLL;
extern InputMapper::InputID INPUT_SCAN_WSO;
extern InputMapper::InputID INPUT_SCREENSHOT;
extern InputMapper::InputID INPUT_SONAR;
extern InputMapper::InputID INPUT_SPEED_TIME;
extern InputMapper::InputID INPUT_SPRINT;
extern InputMapper::InputID INPUT_TIME_BACK;
extern InputMapper::InputID INPUT_TIME_FORWARD;
extern InputMapper::InputID INPUT_TOGGLE_AR;
extern InputMapper::InputID INPUT_TOGGLE_UI;
extern InputMapper::InputID INPUT_UPDATE_FRUSTUM;
extern InputMapper::InputID INPUT_WATER_UPDATE;
extern InputMapper::InputID INPUT_ZOOM;

// Initialize Input IDs At Runtime
extern void initInputs(InputMapper* inputManager);

#endif // Inputs_h__
