/// 
///  InputManager.h
///  Seed of Andromeda
///
///  Created by Frank McCoy
///  Copyright 2014 Regrowth Studios
///  All Rights Reserved
///  
///  Summary:
///  Handles all the user input through Mice, Keyboards and GamePads.
///

#pragma once

#ifndef Input_Manager_h
#define Input_Manager_h

#include <SDL\SDL_events.h>
#include <SDL\SDL_joystick.h>
#include <Vorb/Events.hpp>
#include <Vorb/ui/InputDispatcher.h>

#define DEFAULT_CONFIG_LOCATION "Data/KeyConfig.yml"

/// Handles all the user input through the mouse, keyboard and gamepad.
/// @author Frank McCoy
class InputManager {
public:
    /// All possible axis types.
    enum AxisType {
        DUAL_KEY, ///< A axis which consists of a positive and negative key.
        SINGLE_KEY, ///< A axis which consists of only a positive key.
        JOYSTICK_AXIS, ///< A axis which reads it's values from a joystick - Not implemented.
        JOYSTICK_BUTTON, ///< A axis which reads it's values from a button on a joystick/gamepad - Not implemented.
        NONE ///< A axis which has no type - This should never be used.
    };

    /// The possible event types that can be subscribed to.
    enum EventType {
        UP, ///< Event when a button is released.
        DOWN ///< Event when a button is pressed.
    };

    /// Constructor.
    InputManager();

    /// Declassor.
    ~InputManager();

    /// Get the value of the current axis.

    /// If only one key is to be used in the supplied axis it returns the state of the positive key. 1.0f if true, 0.0f if false.
    /// If both positive and negative keys are to be used, if both or neither key is pressed 0.0f is returned.
    /// If only the positive key is pressed 1.0f is returned.
    /// If only the negative key is pressed -1.0f is returned.
    /// @param axisID: The id of the axis which is being looked up.
    /// @return a value in the range [-1.0, 1.0].
    f32 getAxis(const i32 axisID);

    /// Gets the current state of the supplied axis.

    /// Returns 1.0f if positive key true, 0.0f if positive key is false.
    ///
    /// If given an invalid axisID the function returns false.
    /// @param axisID: The id of the axis which is being looked up.
    /// @return The state of the positive key in the axis.
    bool getKey(const i32 axisID);

    /// Gets if the positive key of the supplied axis was pressed down during the current frame.

    /// If supplied an invalid axis ID the function returns false.
    /// @return true if the positive key was pressed this frame, otherwise false.
    bool getKeyDown(const i32 axisID);

    /// Gets if the positive key in the supplied axis was released this frame.

    /// If supplied an invalid axisID the function returns false.
    /// @return true if positive key was released this frame, otherwise, false.
    bool getKeyUp(const i32 axisID);

    // keys are SDLK_ keys or SDL_BUTTON_
    /// Creates a double key axis.
    /// If the axis already exists the old axisID is returned and no data is modified.
    /// @param axisName: The name of the axis to create.
    /// @param defaultPositiveKey: The default key for the positive portion of the new axis.
    /// @param defaultNegativeKey: The default key for the negative portion of the new axis.
    /// @return The id of the new axis.
    i32 createAxis(const nString& axisName, VirtualKey defaultPositiveKey, VirtualKey defaultNegativeKey); // Double key

    /// Creates a single key axis.
    /// If the axis already exists the old axisID is returned and no data is modified.
    /// @param axisName: The name of the axis to create.
    /// @param defaultKey: The default key(positive) for the new axis.
    /// @return The id of the new axis.
    i32 createAxis(const nString& axisName, VirtualKey defaultKey); // Single key
    // TODO: Add joystick factory methods

    /// Get the positive key of the supplied axis.

    /// If the axis does not exist return UINT32_MAX.
    /// @param axisID: The id of the axis to look up.
    /// @return The id of the positive key of the axis.
    ui32 getPositiveKey(const i32 axisID);

    /// Set the positive key of the supplied axis.

    /// @param axisID: The id of the axis to look up.
    /// @param key: The key to set the axes' positive key to.
    void setPositiveKey(const i32 axisID, VirtualKey key);

    /// Get the negative key of the supplied axis.

    /// If the axis does not exist return UINT32_MAX.
    /// @param axisID: The id of the axis to look up.
    /// @return The negative key of the supplied axis.
    ui32 getNegativeKey(const i32 axisID);

    /// Set the negative key of the supplied axis.

    /// @param axisID: The id of the axis to look up.
    /// @param key: The key to set the axes' negative key to.
    void setNegativeKey(const i32 axisID, VirtualKey key);

    /// Resets the axes' positive key to the default.

    /// @param axisID: The axis to reset to default.
    void setPositiveKeyToDefault(const i32 axisID);

    /// Resets the axes' negative key to the default.

    /// @param axisID: The axis to reset to default.
    void setNegativeKeyToDefault(const i32 axisID);

    /// Gets what tyoe the supplied axis is.

    /// If supplied an invalid axisID the function returns AxisType::NONE.
    /// @param axisID: The axis to look up.
    /// @return The type of axis.
    AxisType getAxisType(const i32 axisID);

    /// Gets the axis ID for the supplied axis.
    
    /// If supplied an invalid axisName the function returns -1.
    /// @param axisName: The name of the axis to look up.
    /// @return The id of the supplied axis.
    i32 getAxisID(const nString& axisName) const;

    /// Reads all the axes stored in a given ini file.
    /// @param filePath: The local path to the file to load axes from.
    void loadAxes(const nString& filePath);

    /// Loads axes from default location ("Data/KeyConfig.ini").
    void loadAxes();

    /// Saves currently stored axes to the given file path.

    /// @param filePath: The local filePath to the file to save the loaded axes into.
    void saveAxes(const nString& filePath);

    /// Saves currently stored axes to the default location ("Data/KeyConfig.ini").
    void saveAxes();

    /// Updates the internal state of all the keys and sends out key pressed and released events.
    void update();

    /// Begins receiving input events from dispatcher
    void startInput();
    /// Stops receiving input events from dispatcher
    void stopInput();

    /// Subscribes a delegate to one of the axes' events.
    /// Returns nullptr if axisID is invalid or eventType is invalid.
    /// @see Event::add
    /// @param axisID: The axis to subscribe the functor to.
    /// @param eventType: The event to subscribe the funtor to.
    /// @param f: The delegate to subscribe to the axes' event.
    /// @return The newly made delegate.
    IDelegate<ui32>* subscribe(const i32 axisID, EventType eventType, IDelegate<ui32>* f);

    /// Subscribes a functor to one of the axes' events.
    /// Returns nullptr if axisID is invalid or eventType is invalid.
    /// @see Event::addFunctor
    /// @param axisID: The axis to subscribe the functor to.
    /// @param eventType: The event to subscribe the funtor to.
    /// @param f: The functor to subscribe to the axes' event.
    /// @return The newly made delegate.
    template<typename F>
    IDelegate<ui32>* subscribeFunctor(const i32 axisID, EventType eventType, F f) {
        return subscribe(axisID, eventType, new Delegate<F, ui32>(f));
    }

    /// Unsubscribes a delegate from a Axes' event.
    /// @see Event::remove
    /// @param axisID: The id of the axis to remove the delegate from
    /// @param eventType: The event to remove the delegate from
    void unsubscribe(const i32 axisID, EventType eventType, IDelegate<ui32>* f);

    /// The data for a single Axis.
    class Axis {
    public:
        nString name; ///< The name of the axis.
        AxisType type; ///< The type of axis.
        VirtualKey defaultPositiveKey; ///< The default positive key.
        VirtualKey defaultNegativeKey; ///< The default negative key.
        VirtualKey positiveKey; ///< The actual positive key.
        VirtualKey negativeKey; ///< The actual negative key.
        SDL_Joystick* joystick; /// The joystick that a JOYSTICK_AXIS would read from.
        i32 joystickAxis; ///< The axis number on the joystick.
        i32 joystickButton; ///< The button number on the joystick.
        Event<ui32> upEvent; ///< The event for when the positive key is released on a SINGLE_KEY axis.
        Event<ui32> downEvent; ///< The event for when the positive key is pressed on a SINGLE_KEY axis.
    };

private:

    const nString _defaultConfigLocation; //"Data/KeyConfig.ini"
    
    std::vector<Axis*> _axes; ///< All the stored axes.
    std::unordered_map<nString, i32> _axisLookup; ///< A map of axis names to axis IDs for quick look up.

    bool _currentKeyStates[VKEY_HIGHEST_VALUE]; ///< The state of the keys and mouse buttons this frame.
    bool _previousKeyStates[VKEY_HIGHEST_VALUE]; ///< The state of the keys and mouse buttons last frame.

    bool m_receivingInput = false; ///< Tracks input reception state
    AutoDelegatePool m_inputHooks; ///< Stores input reception function hooks for deallocation
};

#endif //Input_Manager_h
