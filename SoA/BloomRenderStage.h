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

#define BLOOM_GAUSSIAN_N 10
#define BLOOM_LUMA_THRESHOLD 0.85f
#define BLOOM_GAUSSIAN_VARIANCE 16.0f

#define TASK_WORK 4  // (arbitrary) weight of task
#define TOTAL_TASK 4
#define TOTAL_WORK TOTAL_TASK * TASK_WORK

#define BLOOM_TEXTURE_SLOT_COLOR 0  // texture slot to bind color texture which luma info will be extracted
#define BLOOM_TEXTURE_SLOT_LUMA 0  // texture slot to bind color texture which luma info will be extracted
#define BLOOM_TEXTURE_SLOT_BLUR 1  // texture slot to bind color texture which luma info will be extracted

typedef enum {
	BLOOM_RENDER_STAGE_LUMA,
	BLOOM_RENDER_STAGE_GAUSSIAN_FIRST,
	BLOOM_RENDER_STAGE_GAUSSIAN_SECOND
} BloomRenderStagePass;

class BloomRenderStage : public IRenderStage {
public:

	void init(vui::GameWindow* window, StaticLoadContext& context) override;

	void load(StaticLoadContext& context) override;

	void hook(vg::FullQuadVBO* quad);

	void setStage(BloomRenderStagePass stage);

	void dispose(StaticLoadContext& context) override;

	/// Draws the render stage
	void render(const Camera* camera = nullptr);
private:
	vg::GLProgram m_program_luma, m_program_gaussian_first, m_program_gaussian_second;
	vg::FullQuadVBO* m_quad; ///< For use in processing through data
	BloomRenderStagePass m_stage;

	float gauss(int i, float sigma2);
};

#endif // BloomRenderStage_h__