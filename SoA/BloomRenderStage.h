/// 
///  BloomRenderStage.h
///  Seed of Andromeda
///
///  Created by Isaque Dutra on 2 June 2015
///  Copyright 2015 Regrowth Studios
///  All Rights Reserved
///  
///  This file implements a bloom render stage for
///  MainMenuRenderer.
///

#pragma once

#ifndef BloomRenderStage_h__
#define BloomRenderStage_h__

#include <Vorb/graphics/GLRenderTarget.h>
#include <Vorb/graphics/FullQuadVBO.h>
#include <Vorb/graphics/GLProgram.h>
#include "ShaderLoader.h"
#include "LoadContext.h"

#include "IRenderStage.h"


#define TASK_WORK 4  // (arbitrary) weight of task
#define TOTAL_TASK 1
#define TOTAL_WORK TOTAL_TASK * TASK_WORK
#define BLOOM_TEXTURE_SLOT_COLOR 0  // texture slot to bind color texture which luma info will be extracted

class BloomRenderStage : public IRenderStage {
public:

	void init(vui::GameWindow* window, StaticLoadContext& context) override;

	void load(StaticLoadContext& context) override;

	void hook(vg::FullQuadVBO* quad);

	void setParams();

	void dispose(StaticLoadContext& context) override;

	/// Draws the render stage
	void render(const Camera* camera = nullptr) override;
private:
	vg::GLProgram m_program;
	vg::FullQuadVBO* m_quad; ///< For use in processing through data
};

#endif // BloomRenderStage_h__