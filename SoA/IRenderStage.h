/// 
///  IRenderStage.h
///  Seed of Andromeda
///
///  Created by Ben Arnold on 28 Oct 2014
///  Copyright 2014 Regrowth Studios
///  All Rights Reserved
///  
///  This file provides an abstract interface for a render 
///  stage.
///

#pragma once

#ifndef IRenderStage_h_
#define IRenderStage_h_

#include <Vorb/RPC.h>
#include <Vorb/VorbPreDecl.inl>

class Camera;
class StaticLoadContext;
struct SoaState;
DECL_VUI(class GameWindow)

class IRenderStage {
public:
    /*! @brief Simple fast initialization performed in main game loop.
    *
    * Should increment the amount of expected work to be done in the context.
    * Only simple initialization logic should be performed here
    *
    * @param context: Common loading context.
    */
    virtual void init(vui::GameWindow* window, StaticLoadContext& context VORB_UNUSED) { m_window = window; }

    /*! @brief Invokes core loading logic
    *
    * The loading logic of this render stage is invoked on a separate thread. The work
    * completed should be updated on the context as it is finished.
    *
    * @param context: Common loading context that holds an RPCManager
    */
    virtual void load(StaticLoadContext& context VORB_UNUSED) {}

    /*! @brief Destroys all resources held by this render stage.
    *
    * Called within the game loop, the destruction should be fast and minimal.
    *
    * @param context: Common loading context.
    */
    virtual void dispose(StaticLoadContext& context VORB_UNUSED) {}

    /*! @brief Implementation-defined rendering logic.
    *
    * The rendering code should only concern itself with setting up the shaders, geometry
    * and sending the draw calls. Other state (like draw destinations) are handled externally.
    *
    * @param camera: Viewpoint for the rendering
    */
    virtual void render(const Camera* camera) = 0;

    virtual const volatile bool& isBuilt() const { return m_built; }

    /// Sets the visibility of the stage
    /// @param isVisible: The visibility
    virtual void setActive(bool isVisible) { m_isActive = isVisible; }

    /// Toggles the isVisible field
    virtual void toggleActive() { m_isActive = !m_isActive; }

    /// Check if the stage is visible
    virtual const bool& isActive() const { return m_isActive; }
protected:
    vui::GameWindow* m_window;
    bool m_isActive = true; ///< Determines if the stage should be rendered
    volatile bool m_built = false;
};

#endif // IRenderStage_h_
