#pragma once
#include <SDL\SDL_events.h>
#include <SDL\SDL_joystick.h>

#include "EventManager.h"

enum SDLKey;

#define DEFAULT_CONFIG_LOCATION "Data/KeyConfig.ini"

class InputEventData: public EventData {
public:
    InputEventData(i32 eventID, ui32 key): EventData(eventID), key(key) {}
    ui32 key;
};

class InputManager {
public:
    enum AxisType {
        DUAL_KEY,
        SINGLE_KEY,
        JOYSTICK_AXIS,
        JOYSTICK_BUTTON,
        NONE
    };

    struct Axis {
        nString name;
        AxisType type;
        ui32 defaultPositiveKey;
        ui32 defaultNegativeKey;
        ui32 positiveKey;
        ui32 negativeKey;
        SDL_Joystick* joystick;
        i32 joystickAxis;
        i32 joystickButton;
    };

    InputManager();
    ~InputManager();

    // Returns a value between -1(inclusive) and 1(inclusive)
    // If only one key is to be used in the supplied axis it returns the state of the positive key. 1.0f if true, 0.0f if false
    // If both positive and negative keys are to be used, if both or neither key is pressed 0.0f is returned
    // If only the positive key is pressed 1.0f is returned
    // If only the negative key is pressed -1.0f is returned
    f32 getAxis(const i32 axisID);
    // Returns the state of the positive key in the supplied axis.
    // 1.0f if true, 0.0f if false
    bool getKey(const i32 axisID);
    // Returns if the positive key in a given axis was pressed this frame
    // 1.0f if true, 0.0f if false
    bool getKeyDown(const i32 axisID);
    // Returns if the positive key in a given axis was released this frame
    // 1.0f if true, 0.0f if false
    bool getKeyUp(const i32 axisID);

    // keys are SDLK_ keys or SDL_BUTTON_
    i32 createAxis(const nString& axisName, ui32 defaultPositiveKey, ui32 defaultNegativeKey); // Double key
    i32 createAxis(const nString& axisName, ui32 defaultKey); // Single key
    // TODO: Add joystick factory methods

    ui32 getPositiveKey(const i32 axisID);
    void setPositiveKey(const i32 axisID, ui32 key);
    ui32 getNegativeKey(const int axisID);
    void setNegativeKey(const i32 axisID, ui32 key);
    void setPositiveKeyToDefault(const i32 axisID);
    void setNegativeKeyToDefault(const i32 axisID);
    AxisType getAxisType(const i32 axisID);

    // Returns a value corresponding to the give axis name.
    // if the axis corresponding to the given axisName does not exist -1 is returned
    i32 getAxisID(const nString& axisName) const;

    // Reads all the axes stored in a given ini file
    void loadAxes(const nString& filePath);
    // Loads from default location "Data/KeyConfig.ini"
    void loadAxes();
    // Saves currently stored axes to the give file path
    void saveAxes(const nString& filePath);
    // Saves to default location "Data/KeyConfig.ini"
    void saveAxes();

    // Updates the internal status of all the keys
    void update();
    // Makes the internal map aware that some input event has occurred
    void pushEvent(const SDL_Event& inputEvent);
private:
    i32 getIniKey(const nString& val); //For use with ini file loading

    const nString _defaultConfigLocation; //"Data/KeyConfig.ini"

    std::vector<Axis*> _axes; //All the stored axes
    std::unordered_map<nString, i32> _axisLookup; //A map of axis names to axis IDs for quick look up 
    std::unordered_map<nString, i32> _iniKeys; //For use with ini file loading
    std::map<ui32, bool> _currentKeyStates; //The state of the keys and mouse buttons this frame
    std::map<ui32, bool> _previousKeyStates; //The state of the keys and mouse buttons last frame
};
