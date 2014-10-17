#include "stdafx.h"
#include "FrameBuffer.h"

#include "global.h"
#include "Options.h"


static const GLfloat g_quad_vertex_buffer_data[] = {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
};


FrameBuffer::FrameBuffer(GLint internalFormat, GLenum type, int width, int height, int msaa)
{

    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    cout << "Creating framebuffer of width " << width << " / " << maxTextureSize << endl;
    if (width > maxTextureSize){
        pError("Framebuffer of width" + to_string(width) + " exceeds maximum supported size of " + to_string(maxTextureSize));
    }

    fbWidth = width;
    fbHeight = height;
    frameBufferIDs[FB_DRAW] = 0;
    frameBufferIDs[FB_MSAA] = 0;
    renderedTextureIDs[FB_DRAW] = 0;
    renderedTextureIDs[FB_MSAA] = 0;

    depthTextureIDs[FB_DRAW] = 0;
    depthTextureIDs[FB_MSAA] = 0;

    vertexBufferID = 0;
    elementBufferID = 0;

    //Initialize frameBuffer
    glGenFramebuffers(1, &(frameBufferIDs[FB_DRAW]));
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferIDs[FB_DRAW]);

    // The texture we're going to render to
    glGenTextures(1, &renderedTextureIDs[FB_DRAW]);

    glBindTexture(GL_TEXTURE_2D, renderedTextureIDs[FB_DRAW]);

    // Poor filtering. Needed !
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Give an empty image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_BGRA, type, NULL);
    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTextureIDs[FB_DRAW], 0);

    // The depth buffer
    glGenTextures(1, &depthTextureIDs[FB_DRAW]);

    glBindTexture(GL_TEXTURE_2D, depthTextureIDs[FB_DRAW]);

    glBindTexture(GL_TEXTURE_2D, depthTextureIDs[FB_DRAW]);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTextureIDs[FB_DRAW], 0);


    // Set the list of draw buffers.

    GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };

    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
    
    checkErrors();

    if (msaa > 0){
        //Initialize frameBuffer
        glGenFramebuffers(1, &(frameBufferIDs[FB_MSAA]));
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferIDs[FB_MSAA]);

        // The texture we're going to render to
        glGenTextures(1, &renderedTextureIDs[FB_MSAA]);

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderedTextureIDs[FB_MSAA]);
        
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, msaa, internalFormat, width, height, 0);

        GLuint err = glGetError();
        if (err != GL_NO_ERROR){
            showMessage(to_string((int)err));
        }

        // Set "renderedTexture" as our colour attachement #0
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, renderedTextureIDs[FB_MSAA], 0);
        
        // The depth buffer
        glGenTextures(1, &depthTextureIDs[FB_MSAA]);

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTextureIDs[FB_MSAA]);

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTextureIDs[FB_MSAA]);

    //    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, msaa, GL_DEPTH_COMPONENT32, width, height, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depthTextureIDs[FB_MSAA], 0);

        glDrawBuffers(1, DrawBuffers);
        glReadBuffer(GL_COLOR_ATTACHMENT0); 


        checkErrors("MSAA Frame Buffer");

    }

    glGenVertexArrays(1, &quadVertexArrayID);

    glBindVertexArray(quadVertexArrayID);


    glGenBuffers(1, &quadVertexBufferID);

    glBindBuffer(GL_ARRAY_BUFFER, quadVertexBufferID);

    glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

    unBind();
}

FrameBuffer::~FrameBuffer()
{
    if (renderedTextureIDs[FB_DRAW] != 0){
        glDeleteTextures(1, &renderedTextureIDs[FB_DRAW]);
        renderedTextureIDs[FB_DRAW] = 0;
    }
    if (renderedTextureIDs[FB_MSAA] != 0){
        glDeleteTextures(1, &renderedTextureIDs[FB_MSAA]);
        renderedTextureIDs[FB_MSAA] = 0;
    }
    if (depthTextureIDs[FB_DRAW] != 0){
        glDeleteTextures(1, &depthTextureIDs[FB_DRAW]);
        depthTextureIDs[FB_DRAW] = 0;
    }
    if (depthTextureIDs[FB_MSAA] != 0){
        glDeleteTextures(1, &depthTextureIDs[FB_MSAA]);
        depthTextureIDs[FB_MSAA] = 0;
    }
    if (frameBufferIDs[FB_DRAW] != 0){
        glDeleteFramebuffers(1, &(frameBufferIDs[FB_DRAW]));
        frameBufferIDs[FB_DRAW] = 0;
    }
    if (frameBufferIDs[FB_MSAA] != 0){
        glDeleteFramebuffers(1, &(frameBufferIDs[FB_MSAA]));
        frameBufferIDs[FB_MSAA] = 0;
    }
    if (quadVertexArrayID != 0){
        glDeleteVertexArrays(1, &quadVertexArrayID);
        quadVertexArrayID = 0;
    }
    if (vertexBufferID != 0){
        glDeleteBuffers(1, &vertexBufferID);
        vertexBufferID = 0;
    }
    if (elementBufferID != 0){
        glDeleteBuffers(1, &elementBufferID);
        elementBufferID = 0;
    }
    unBind();
}

void FrameBuffer::bind(int msaa)
{
    if (msaa > 0){
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferIDs[FB_MSAA]);
    }
    else{
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferIDs[FB_DRAW]);
    }
    glViewport(0, 0, fbWidth, fbHeight); // Render on the whole framebuffer, complete from the lower left corner to the upper right
}

void FrameBuffer::unBind()
{
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, graphicsOptions.windowWidth, graphicsOptions.windowHeight);
}

void FrameBuffer::checkErrors(string name)
{
    checkGlError(name);
    // Always check that our framebuffer is ok
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE){
        string err = "Unknown Error " + to_string((int)status);
        if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT){
            err = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
        }
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT){
            err = "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT";
        }
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT){
            err = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
        }
        else if (status == GL_FRAMEBUFFER_UNSUPPORTED){
            err = "GL_FRAMEBUFFER_UNSUPPORTED";
        }
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER){
            err = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
        }
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE){
            err = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
        }
        pError(name + " did not properly initialize: " + err);
        exit(414);
    }
}

void FrameBuffer::draw()
{
    if (vertexBufferID == 0){
        glGenBuffers(1, &vertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(wholeScreenVertices), wholeScreenVertices, GL_STATIC_DRAW);

        glGenBuffers(1, &elementBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(boxDrawIndices), boxDrawIndices, GL_STATIC_DRAW);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //Solid mode


    //need to blit the MSAA to a normal texture
    if (graphicsOptions.msaa > 0){
        glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBufferIDs[FB_MSAA]);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBufferIDs[FB_DRAW]);
        glBlitFramebuffer(0, 0, fbWidth, fbHeight, 0, 0, fbWidth, fbHeight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderedTextureIDs[FB_DRAW]);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, graphicsOptions.windowWidth, graphicsOptions.windowHeight); // Render on the whole screen, complete from the lower left corner to the upper right

//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set our "renderedTexture" sampler to user Texture Unit 0
    if (shaderMode & FB_SHADER_MOTIONBLUR){
        motionBlurShader.Bind();
        glUniform1i(motionBlurShader.texID, 0);
        glUniform1i(motionBlurShader.depthID, 1);
        glUniform1f(motionBlurShader.gammaID, 1.0f / graphicsOptions.gamma);
        glUniform1f(motionBlurShader.fExposureID, graphicsOptions.hdrExposure);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthTextureIDs[FB_DRAW]);
        
        glUniformMatrix4fv(motionBlurShader.prevVPID, 1, GL_FALSE, &motionBlurShader.oldVP[0][0]);
        glUniformMatrix4fv(motionBlurShader.inverseVPID, 1, GL_FALSE, &motionBlurShader.newInverseVP[0][0]);
        glUniform1i(motionBlurShader.numSamplesID, (int)graphicsOptions.motionBlur);
        //glUniform1f(hdrShader.averageLuminanceID, luminance);
    }
    else{
        hdrShader.Bind();
        glUniform1i(hdrShader.texID, 0);
        glUniform1f(hdrShader.gammaID, 1.0f / graphicsOptions.gamma);
        glUniform1f(hdrShader.fExposureID, graphicsOptions.hdrExposure);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);

    // 1rst attribute buffer : vertices
    glVertexAttribPointer(
        0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        2,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
        );

    // Draw the triangles !
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferID);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    if (shaderMode & FB_SHADER_MOTIONBLUR){
        motionBlurShader.UnBind();
    }
    else{
        hdrShader.UnBind();
    }

    if (drawMode == 0){
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //Solid mode
    }
    else if (drawMode == 1){
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); //Point mode
    }
    else if (drawMode == 2){
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Wireframe mode
    }
}