#include "stdafx.h"
#include "LiquidVoxelRenderStage.h"


LiquidVoxelRenderStage::LiquidVoxelRenderStage()
{
}


LiquidVoxelRenderStage::~LiquidVoxelRenderStage()
{
}

void LiquidVoxelRenderStage::setState(vg::FrameBuffer* frameBuffer /*= nullptr*/)
{
    throw std::logic_error("The method or operation is not implemented.");
}

void LiquidVoxelRenderStage::draw()
{
    throw std::logic_error("The method or operation is not implemented.");

    //glProgram->use();

    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, waterTexture);
    //glUniform1i(glProgram->getUniform("samplerName1"), 0);
    //glActiveTexture(GL_TEXTURE1);
    //glBindTexture(GL_TEXTURE_2D, bricksTexture);
    //glUniform1i(glProgram->getUniform("samplerName2"), 1);

    //// DRAW STUFF HERE

    //glProgram->unuse();
}

bool LiquidVoxelRenderStage::isVisible()
{
    throw std::logic_error("The method or operation is not implemented.");
}
