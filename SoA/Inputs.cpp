#include "stdafx.h"
#include "Inputs.h"

#include "GameManager.h"

// Input Commands Sorted Alphabetically
InputMapper::InputID INPUT_BACKWARD = -1;
InputMapper::InputID INPUT_BLOCK_DRAG = -1;
InputMapper::InputID INPUT_BLOCK_SCANNER = -1;
InputMapper::InputID INPUT_CROUCH = -1;
InputMapper::InputID INPUT_CYCLE_COLOR_FILTER = -1;
InputMapper::InputID INPUT_DEBUG = -1;
InputMapper::InputID INPUT_DRAW_MODE = -1;
InputMapper::InputID INPUT_EXIT = -1;
InputMapper::InputID INPUT_FLASH_LIGHT = -1;
InputMapper::InputID INPUT_FLY = -1;
InputMapper::InputID INPUT_FORWARD = -1;
InputMapper::InputID INPUT_GRID = -1;
InputMapper::InputID INPUT_HUD = -1;
InputMapper::InputID INPUT_INVENTORY = -1;
InputMapper::InputID INPUT_JUMP = -1;
InputMapper::InputID INPUT_LEFT = -1;
InputMapper::InputID INPUT_LEFT_ROLL = -1;
InputMapper::InputID INPUT_MARKER = -1;
InputMapper::InputID INPUT_MEGA_SPEED = -1;
InputMapper::InputID INPUT_MOUSE_LEFT = -1;
InputMapper::InputID INPUT_MOUSE_RIGHT = -1;
InputMapper::InputID INPUT_NIGHT_VISION = -1;
InputMapper::InputID INPUT_NIGHT_VISION_RELOAD = -1;
InputMapper::InputID INPUT_PAUSE = -1;
InputMapper::InputID INPUT_PHYSICS_BLOCK_UPDATES = -1;
InputMapper::InputID INPUT_PLANET_DRAW_MODE = -1;
InputMapper::InputID INPUT_PLANET_ROTATION = -1;
InputMapper::InputID INPUT_RANDOM_DEBUG = -1;
InputMapper::InputID INPUT_RELOAD_BLOCKS = -1;
InputMapper::InputID INPUT_RELOAD_SHADERS = -1;
InputMapper::InputID INPUT_RELOAD_SYSTEM = -1;
InputMapper::InputID INPUT_RELOAD_TEXTURES = -1;
InputMapper::InputID INPUT_RELOAD_UI = -1;
InputMapper::InputID INPUT_RIGHT = -1;
InputMapper::InputID INPUT_RIGHT_ROLL = -1;
InputMapper::InputID INPUT_SCAN_WSO = -1;
InputMapper::InputID INPUT_SCREENSHOT = -1;
InputMapper::InputID INPUT_SONAR = -1;
InputMapper::InputID INPUT_SPEED_TIME = -1;
InputMapper::InputID INPUT_SPRINT = -1;
InputMapper::InputID INPUT_TIME_BACK = -1;
InputMapper::InputID INPUT_TIME_FORWARD = -1;
InputMapper::InputID INPUT_TOGGLE_AR = -1;
InputMapper::InputID INPUT_TOGGLE_UI = -1;
InputMapper::InputID INPUT_UPDATE_FRUSTUM = -1;
InputMapper::InputID INPUT_WATER_UPDATE = -1;
InputMapper::InputID INPUT_ZOOM = -1;


// Reduce Some Code
#define CREATE_INPUT(ID,KEY,VAR) \
    VAR = inputManager->createInput(#ID, KEY);

// Generate Input Handles
void initInputs(InputMapper* inputManager) {
 //   CREATE_INPUT(Random Debug, VKEY_6, INPUT_RANDOM_DEBUG);

    // The Greatest Input In The Cosmos
 //   CREATE_INPUT(Pause, VKEY_ESCAPE, INPUT_PAUSE);

    // Game Information
 //   CREATE_INPUT(Debug, VKEY_H, INPUT_DEBUG);
 //   CREATE_INPUT(Inventory, VKEY_TAB, INPUT_INVENTORY);
 //   CREATE_INPUT(HUD, VKEY_T, INPUT_HUD);

    // Visual Aid
 //   CREATE_INPUT(Zoom, VKEY_RCTRL, INPUT_ZOOM);
 //   CREATE_INPUT(Sonar, VKEY_R, INPUT_SONAR);
 //   CREATE_INPUT(Flash Light, VKEY_L, INPUT_FLASH_LIGHT);
 //   CREATE_INPUT(Night Vision, VKEY_N, INPUT_NIGHT_VISION);

    // Refreshing Functions
 //   CREATE_INPUT(Reload Textures, VKEY_F4, INPUT_RELOAD_TEXTURES);
 //   CREATE_INPUT(Reload Blocks, VKEY_F6, INPUT_RELOAD_BLOCKS);
    CREATE_INPUT(Reload Shaders, VKEY_F11, INPUT_RELOAD_SHADERS);
    CREATE_INPUT(Reload System, VKEY_F10, INPUT_RELOAD_SYSTEM);
    CREATE_INPUT(Reload UI, VKEY_F5, INPUT_RELOAD_UI);
 //   CREATE_INPUT(Reload Night Vision, VKEY_F3, INPUT_NIGHT_VISION_RELOAD);

    // Visual Debugging
 //   CREATE_INPUT(Grid Toggle, VKEY_G, INPUT_GRID);
 //   CREATE_INPUT(Draw Mode, VKEY_M, INPUT_DRAW_MODE);
 //   CREATE_INPUT(Planet Draw Mode, VKEY_J, INPUT_PLANET_DRAW_MODE);
 //   CREATE_INPUT(Update Frustum, VKEY_U, INPUT_UPDATE_FRUSTUM);
    CREATE_INPUT(Cycle Color Filter, VKEY_C, INPUT_CYCLE_COLOR_FILTER);

    // Movement
  //  CREATE_INPUT(Fly, VKEY_F, INPUT_FLY);
  //  CREATE_INPUT(Sprint, VKEY_LSHIFT, INPUT_SPRINT);
  //  CREATE_INPUT(Crouch, VKEY_LCTRL, INPUT_CROUCH);
  //  CREATE_INPUT(Mega Speed, VKEY_LSHIFT, INPUT_MEGA_SPEED);
  //  CREATE_INPUT(Jump, VKEY_SPACE, INPUT_JUMP);
  //  CREATE_INPUT(Forward, VKEY_W, INPUT_FORWARD);
 //   CREATE_INPUT(Left, VKEY_A, INPUT_LEFT);
  //  CREATE_INPUT(Right, VKEY_D, INPUT_RIGHT);
  //  CREATE_INPUT(Backward, VKEY_S, INPUT_BACKWARD);
  //  CREATE_INPUT(Right Roll, VKEY_E, INPUT_RIGHT_ROLL);
  //  CREATE_INPUT(Left Roll, VKEY_Q, INPUT_LEFT_ROLL);

    // Gameplay
  //  CREATE_INPUT(Marker, VKEY_C, INPUT_MARKER);
  //  CREATE_INPUT(Scan WSO, VKEY_LEFTBRACKET, INPUT_SCAN_WSO);

    // Physics
  //  CREATE_INPUT(Water Update, VKEY_N, INPUT_WATER_UPDATE);
  //  CREATE_INPUT(Update Physics Blocks, VKEY_P, INPUT_PHYSICS_BLOCK_UPDATES);

    // Mouse Buttons
    CREATE_INPUT(Mouse Right, (VirtualKey)SDL_BUTTON_RIGHT, INPUT_MOUSE_RIGHT);
    CREATE_INPUT(Mouse Left, (VirtualKey)SDL_BUTTON_LEFT, INPUT_MOUSE_LEFT);
    
    // Block Utilities
 //   CREATE_INPUT(Block Scanner, VKEY_Q, INPUT_BLOCK_SCANNER);
 //   CREATE_INPUT(Block Select, VKEY_B, INPUT_BLOCK_DRAG);

    // Main Menu
    CREATE_INPUT(Exit, VKEY_ESCAPE, INPUT_EXIT);
    CREATE_INPUT(Toggle UI, VKEY_U, INPUT_TOGGLE_UI);
    CREATE_INPUT(Toggle AR, VKEY_A, INPUT_TOGGLE_AR);
    CREATE_INPUT(Speed Time, VKEY_LCTRL, INPUT_SPEED_TIME);
    CREATE_INPUT(Take Screenshot, VKEY_F2, INPUT_SCREENSHOT);
    CREATE_INPUT(Time Back, VKEY_LEFT, INPUT_TIME_BACK);
    CREATE_INPUT(Time Forward, VKEY_RIGHT, INPUT_TIME_FORWARD);
}