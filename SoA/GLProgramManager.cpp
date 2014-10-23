#include "stdafx.h"
#include "GLProgramManager.h"


namespace vorb {
namespace core {


GLProgramManager::GLProgramManager() {
    // Empty
}


GLProgramManager::~GLProgramManager() {
    // Empty
}


void GLProgramManager::addProgram(nString shaderName, cString vertexPath, cString fragmentPath, const std::vector<nString>* attr /* = nullptr */) {

    std::cout << "Creating GL Shader Program " << shaderName << std::endl;

    // Check to see if the program is already made
    auto it = _programs.find(shaderName);
    if (it != _programs.end()) {
        std::cerr << "Error: Already loaded shader " << shaderName << ". Reloading...\n";
        delete it->second;
    }
    // Allocate the program
    GLProgram* newProgram = new GLProgram(true);

    // Create the vertex shader
    newProgram->addShaderFile(ShaderType::VERTEX, vertexPath);

    // Create the fragment shader
    newProgram->addShaderFile(ShaderType::FRAGMENT, fragmentPath);

    // Set the attributes
    if (attr) {
        newProgram->setAttributes(*attr);
    }

    // Link the program
    newProgram->link();

    // Init the uniforms
    newProgram->initUniforms();

    // Init the attributes
    newProgram->initAttributes();

    // Add program to the map
    _programs[shaderName] = newProgram;
}

void GLProgramManager::addProgram(nString shaderName, GLProgram* program) {
    // Check to see if the program is already made
    auto it = _programs.find(shaderName);
    if (it != _programs.end()) {
        std::cerr << "Error: Already loaded shader " << shaderName << ". Reloading...\n";
        delete it->second;
    }

    // Add program to the map
    _programs[shaderName] = program;
}

GLProgram* GLProgramManager::getProgram(nString shaderName) {
    auto it = _programs.find(shaderName);
    if (it != _programs.end()) {
        return it->second;
    }
    return nullptr;
}

void GLProgramManager::destroy() {

    for (auto prog : _programs) {
        prog.second->destroy();
    }

    std::unordered_map<nString, GLProgram*>().swap(_programs);
}

}
}