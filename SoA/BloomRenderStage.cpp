#include "stdafx.h"

#include "BloomRenderStage.h"

#include <sstream>
#include <Vorb/graphics/GLRenderTarget.h>
#include <Vorb/graphics/GLProgram.h>
#include "ShaderLoader.h"
#include "LoadContext.h"
#include "Errors.h"
#include "Vorb/ui/GameWindow.h"


float BloomRenderStage::gauss(int i, float sigma2) {
		return 1.0 / std::sqrt(2 * 3.14159265 * sigma2) * std::exp(-(i*i) / (2 * sigma2));
}


void BloomRenderStage::init(vui::GameWindow* window, StaticLoadContext& context) {
	
	IRenderStage::init(window, context);
	context.addAnticipatedWork(TOTAL_WORK, TOTAL_TASK);

}

void BloomRenderStage::load(StaticLoadContext& context) {
	// luma
	context.addTask([&](Sender, void*) {
		m_program_luma = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert", "Shaders/PostProcessing/BloomLuma.frag");
		m_program_luma.use();
		glUniform1i(m_program_luma.getUniform("unTexColor"), BLOOM_TEXTURE_SLOT_COLOR);
		glUniform1f(m_program_luma.getUniform("unLumaThresh"), BLOOM_LUMA_THRESHOLD);
		m_program_luma.unuse();
		context.addWorkCompleted(TOTAL_TASK);
	}, false);

	// gaussian first pass
	context.addTask([&](Sender, void*) {
		m_program_gaussian_first = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert", "Shaders/PostProcessing/BloomGaussianFirst.frag");
		m_program_gaussian_first.use();
		glUniform1i(m_program_gaussian_first.getUniform("unTexLuma"), BLOOM_TEXTURE_SLOT_LUMA);
		glUniform1i(m_program_gaussian_first.getUniform("unHeight"), m_window->getHeight());
		glUniform1i(m_program_gaussian_first.getUniform("unGaussianN"), BLOOM_GAUSSIAN_N);
		m_program_gaussian_first.unuse();
		context.addWorkCompleted(TOTAL_TASK);
	}, true);

	// gaussian second pass
	context.addTask([&](Sender, void*) {
		m_program_gaussian_second = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert", "Shaders/PostProcessing/BloomGaussianSecond.frag");
		m_program_gaussian_second.use();
		glUniform1i(m_program_gaussian_second.getUniform("unTexColor"), BLOOM_TEXTURE_SLOT_COLOR);
		glUniform1i(m_program_gaussian_second.getUniform("unTexBlur"), BLOOM_TEXTURE_SLOT_BLUR);
		glUniform1i(m_program_gaussian_second.getUniform("unWidth"), m_window->getWidth());
		glUniform1i(m_program_gaussian_second.getUniform("unGaussianN"), BLOOM_GAUSSIAN_N);
		m_program_gaussian_second.unuse();
		context.addWorkCompleted(TOTAL_TASK);
	}, true);

	// calculate gaussian weights
	
	context.addTask([&](Sender, void*) {
		float weights[BLOOM_GAUSSIAN_N], sum;
		weights[0] = gauss(0, BLOOM_GAUSSIAN_VARIANCE);
		sum = weights[0];
		for (int i = 0; i < BLOOM_GAUSSIAN_N; i++) {
			weights[i] = gauss(i, BLOOM_GAUSSIAN_VARIANCE);
			sum += 2 * weights[i];
		}
		for (int i = 0; i < BLOOM_GAUSSIAN_N; i++) {
			weights[i] = weights[i] / sum;
		}
		m_program_gaussian_first.use();
		glUniform1fv(m_program_gaussian_first.getUniform("unWeight[0]"), BLOOM_GAUSSIAN_N, weights);
		m_program_gaussian_first.unuse();
		m_program_gaussian_second.use();
		glUniform1fv(m_program_gaussian_second.getUniform("unWeight[0]"), BLOOM_GAUSSIAN_N, weights);
		m_program_gaussian_second.unuse();

		context.addWorkCompleted(TOTAL_TASK);
	}, false);

}

void BloomRenderStage::hook(vg::FullQuadVBO* quad) {

	m_quad = quad;

}

void BloomRenderStage::setStage(BloomRenderStagePass stage) {
	m_stage = stage;
}

void BloomRenderStage::dispose(StaticLoadContext& context) {
	m_program_luma.dispose();
	m_program_gaussian_first.dispose();
	m_program_gaussian_second.dispose();
}

void BloomRenderStage::render(const Camera* camera) {

	// luma
	if (m_stage == BLOOM_RENDER_STAGE_LUMA) {
		m_program_luma.use();
		checkGlError("MainMenuRenderPipeline::render()");

		m_program_luma.enableVertexAttribArrays();
		checkGlError("MainMenuRenderPipeline::render()");

		glDisable(GL_DEPTH_TEST);

		checkGlError("MainMenuRenderPipeline::render()");
		m_quad->draw();
		checkGlError("MainMenuRenderPipeline::render()");

		glEnable(GL_DEPTH_TEST);
		checkGlError("MainMenuRenderPipeline::render()");

		m_program_luma.disableVertexAttribArrays();
		checkGlError("MainMenuRenderPipeline::render()");
		m_program_luma.unuse();
		checkGlError("MainMenuRenderPipeline::render()");
	}
	// first gaussian pass
	if (m_stage == BLOOM_RENDER_STAGE_GAUSSIAN_FIRST) {
		m_program_gaussian_first.use();
		m_program_gaussian_first.enableVertexAttribArrays();

		glDisable(GL_DEPTH_TEST);
		m_quad->draw();
		glEnable(GL_DEPTH_TEST);

		m_program_gaussian_first.disableVertexAttribArrays();
		m_program_gaussian_first.unuse();
	}
	// second gaussian pass
	if (m_stage == BLOOM_RENDER_STAGE_GAUSSIAN_SECOND) {
		m_program_gaussian_second.use();
		m_program_gaussian_second.enableVertexAttribArrays();

		glDisable(GL_DEPTH_TEST);
		m_quad->draw();
		glEnable(GL_DEPTH_TEST);

		m_program_gaussian_second.disableVertexAttribArrays();
		m_program_gaussian_second.unuse();
	}

}

