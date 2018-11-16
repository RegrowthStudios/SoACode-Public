/// 
///  InputMapper.h
///  Seed of Andromeda
///
///  Created by Frank McCoy
///  Refactored by Ben Arnold on Mar 25 2015
///  Copyright 2014 Regrowth Studios
///  MIT License
///  
///  Summary:
///  Handles mapping of input for keys and buttons.
///

#pragma once

#ifndef Input_Manager_h
#define Input_Manager_h

#include <Vorb/Event.hpp>
#include <Vorb/ui/InputDispatcher.h>

#define INPUTMAPPER_DEFAULT_CONFIG_LOCATION "Data/KeyConfig.yml"

/// Handles all the user input through the mouse, keyboard and gamepad.
/// @author Frank McCoy and Ben Arnold
class InputMapper {
public:
    typedef Event<ui32>::Subscriber Listener;
    typedef i32 InputID;

    /// Constructor.
    InputMapper();
    /// Destructor.
    ~InputMapper();

    /// The data for a single Input.
    class Input {
    public:
        Input(InputID ID, const nString& nm, VirtualKey defKey, InputMapper* parent) :
        id(ID),
        name(nm),
        defaultKey(defKey),
        key(defKey),
        upEvent(parent),
        downEvent(parent){
            // Empty
        }
        InputID id;
        nString name; ///< The name of the input.
        VirtualKey defaultKey; ///< The default key.
        VirtualKey key; ///< The actual key.
        Event<ui32> upEvent; ///< The event for when the key is released
        Event<ui32> downEvent; ///< The event for when the key is pressed
    };
    typedef std::vector<Input> InputList;
    typedef std::unordered_map<nString, InputID> InputMap;

    /// Returns the state of an input
    /// @param id: The id of the input which is being looked up.
    /// @return The state of the positive key in the input.
    bool getInputState(const InputID id);

    /// Creates a single key input.
    /// If the input already exists the old ID is returned and no data is modified.
    /// @param inputName: The name of the input to create.
    /// @param defaultKey: The default key(positive) for the new input.
    /// @return The id of the new input.
    InputID createInput(const nString& inputName, VirtualKey defaultKey); // Single key
  

    /// Get the positive key of the supplied input.
    /// If the input does not exist return UINT32_MAX.
    /// @param id: The id of the input to look up.
    /// @return The id of the positive key of the input.
    VirtualKey getKey(const InputID id);

    /// Set the positive key of the supplied input.
    /// @param id: The id of the input to look up.
    /// @param key: The key to set the keys' positive key to.
    void setKey(const InputID id, VirtualKey key);

    /// Resets the axes' positive key to the default.
    /// @param id: The input to reset to default.
    void setKeyToDefault(const InputID id);

    /// Gets the input ID for the supplied input.
    /// If supplied an invalid inputName the function returns -1.
    /// @param inputName: The name of the input to look up.
    /// @return The id of the supplied input.
    InputID getInputID(const nString& inputName) const;

    /// Reads all the axes stored in a given ini file.
    /// @param filePath: The local path to the file to load axes from.
    void loadInputs(const nString& filePath = INPUTMAPPER_DEFAULT_CONFIG_LOCATION);

    /// Saves currently stored axes to the given file path.
    /// @param filePath: The local filePath to the file to save the loaded axes into.
    void saveInputs(const nString& filePath = INPUTMAPPER_DEFAULT_CONFIG_LOCATION);

    /// Begins receiving input events from dispatcher
    void startInput();
    /// Stops receiving input events from dispatcher
    void stopInput();

    const bool& isRecievingInput() const { return m_receivingInput; }

    /// Gets the input associated with the InputID
    Input& get(InputID i) {
        return m_inputs[i];
    }
    Input& operator[](InputID i) {
        return m_inputs[i];
    }

    const InputMap& getInputLookup() const { return m_inputLookup; }

private:
    void onMouseButtonDown(Sender, const vui::MouseButtonEvent& e);
    void onMouseButtonUp(Sender, const vui::MouseButtonEvent& e);
    void onKeyDown(Sender, const vui::KeyEvent& e);
    void onKeyUp(Sender, const vui::KeyEvent& e);
   
    InputList m_inputs; ///< All the stored axes.
    InputMap m_inputLookup; ///< A map of input names to input IDs for quick look up.
    std::unordered_map<VirtualKey, std::vector<InputID> > m_keyCodeMap; ///< Map of keycodes to active input
    
    /// Assuming vui::MouseButton wont change...
    bool m_keyStates[VKEY_HIGHEST_VALUE + (ui32)vui::MouseButton::X2]; ///< The state of the keys and mouse buttons this frame.
  
    bool m_receivingInput = false; ///< Tracks input reception state
    AutoDelegatePool m_inputHooks; ///< Stores input reception function hooks for deallocation
};

#endif //Input_Manager_h
