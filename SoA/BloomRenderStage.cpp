#include "stdafx.h"

#include "BloomRenderStage.h"

#include <Vorb/graphics/GLRenderTarget.h>
#include <Vorb/graphics/GLProgram.h>
#include "ShaderLoader.h"
#include "LoadContext.h"

void BloomRenderStage::init(vui::GameWindow* window, StaticLoadContext& context) {
	
	IRenderStage::init(window, context);
	context.addAnticipatedWork(TOTAL_WORK, TOTAL_TASK);

}

void BloomRenderStage::load(StaticLoadContext& context) {

	context.addTask([&](Sender, void*) {		
		m_program = ShaderLoader::createProgramFromFile("Shaders/PostProcessing/PassThrough.vert", "Shaders/PostProcessing/Bloom.frag");
		m_program.use();
		glUniform1i(m_program.getUniform("unTexColor"), BLOOM_TEXTURE_SLOT_COLOR);
		m_program.unuse();
		context.addWorkCompleted(TOTAL_WORK);
	}, false);

}

void BloomRenderStage::hook(vg::FullQuadVBO* quad) {

	m_quad = quad;

}

void BloomRenderStage::setParams() {

}

void BloomRenderStage::dispose(StaticLoadContext& context) {
	m_program.dispose();
}

void BloomRenderStage::render(const Camera* camera) {

	m_program.use();
	m_program.enableVertexAttribArrays();

	glDisable(GL_DEPTH_TEST);
	m_quad->draw();
	glEnable(GL_DEPTH_TEST);

	m_program.disableVertexAttribArrays();
	m_program.unuse();

}

