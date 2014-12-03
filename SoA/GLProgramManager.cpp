#include "stdafx.h"
#include "GLProgramManager.h"

#include "Errors.h"
#include "GLEnums.h"

vg::GLProgramManager::GLProgramManager() {
    // Empty
}


vg::GLProgramManager::~GLProgramManager() {
    // Empty
}


void vg::GLProgramManager::addProgram(nString shaderName, cString vertexPath, cString fragmentPath, const std::vector<nString>* attr /* = nullptr */, cString defines /*= nullptr*/) {

    bool rebuild = true;

    GLProgram* newProgram;

    do {
        std::cout << "Creating GL Shader Program " << shaderName << std::endl;

        // Check to see if the program is already made
        auto it = _programs.find(shaderName);
        if (it != _programs.end()) {
            std::cerr << "Error: Already loaded shader " << shaderName << ". Reloading...\n";
            delete it->second;
        }
        // Allocate the program
        newProgram = new GLProgram(true);

        IOManager iom;
        const cString code;

        // Create the vertex shader
        ShaderSource srcVert;
        srcVert.stage = vg::ShaderType::VERTEX_SHADER;
        if (defines) srcVert.sources.push_back(defines);
        code = iom.readFileToString(vertexPath);
        srcVert.sources.push_back(code);
        if (!newProgram->addShader(srcVert)) {
            showMessage("Vertex shader for " + shaderName + " failed to compile. Check command prompt for errors. After you fix errors, press OK to try again.");
            delete[] code;
            delete newProgram;
            continue;
        }
        delete[] code;

        // Create the fragment shader
        ShaderSource srcFrag;
        srcFrag.stage = vg::ShaderType::FRAGMENT_SHADER;
        if (defines) srcFrag.sources.push_back(defines);
        code = iom.readFileToString(fragmentPath);
        srcFrag.sources.push_back(iom.readFileToString(fragmentPath));
        if (!newProgram->addShader(srcFrag)) {
            showMessage("Fragment shader for " + shaderName + " failed to compile. Check command prompt for errors. After you fix errors, press OK to try again.");
            delete[] code;
            delete newProgram;
            continue;
        }
        delete[] code;

        // Set the attributes
        if (attr) {
            newProgram->setAttributes(*attr);
        }

        // Link the program
        if (newProgram->link()) {
            // If it linked successfully, then we don't rebuild
            rebuild = false; 
        } else {
            // If it failed to link, print error message and try to rebuild
            showMessage("Shader Program " + shaderName + " failed to link. Check command prompt for errors. After you fix errors, press OK to try again.");
            delete newProgram;
        }

    } while (rebuild);
    // Init the uniforms
    newProgram->initUniforms();

    // Init the attributes
    newProgram->initAttributes();

    // Add program to the map
    _programs[shaderName] = newProgram;
}

void vg::GLProgramManager::addProgram(nString shaderName, GLProgram* program) {
    // Check to see if the program is already made
    auto it = _programs.find(shaderName);
    if (it != _programs.end()) {
        std::cerr << "Error: Already loaded shader " << shaderName << ". Reloading...\n";
        delete it->second;
    }

    // Add program to the map
    _programs[shaderName] = program;
}

vg::GLProgram* vg::GLProgramManager::getProgram(nString shaderName) const {
    auto it = _programs.find(shaderName);
    if (it != _programs.end()) {
        return it->second;
    }
    return nullptr;
}

void vg::GLProgramManager::destroy() {

    for (auto prog : _programs) {
        prog.second->destroy();
        delete prog.second;
    }

    std::unordered_map<nString, GLProgram*>().swap(_programs);
}
