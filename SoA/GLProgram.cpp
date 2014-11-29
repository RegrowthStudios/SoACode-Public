#include "stdafx.h"
#include "GLProgram.h"

#include "IOManager.h"

// Used for querying attribute and uniforms variables within a program
#define PROGRAM_VARIABLE_IGNORED_PREFIX "gl_"
#define PROGRAM_VARIABLE_IGNORED_PREFIX_LEN 3
#define PROGRAM_VARIABLE_MAX_LENGTH 1024

const vg::ShaderLanguageVersion vg::DEFAULT_SHADING_LANGUAGE_VERSION = vg::ShaderLanguageVersion(
    GL_PROGRAM_DEFAULT_SHADER_VERSION_MAJOR,
    GL_PROGRAM_DEFAULT_SHADER_VERSION_MINOR,
    GL_PROGRAM_DEFAULT_SHADER_VERSION_REVISION
);

vg::GLProgram* vg::GLProgram::_programInUse = nullptr;

vg::GLProgram::GLProgram(bool init /*= false*/) {
    if (init) this->init();
}

vg::GLProgram::~GLProgram() {
    destroy();
}

void vg::GLProgram::init() {
    if (getIsCreated()) return;
    _isLinked = false;
    _id = glCreateProgram();
}
void vg::GLProgram::destroy() {
    // Delete the shaders
    if (_idVS) {
        glDeleteShader(_idVS);
        _idVS = 0;
    }
    if (_idFS) {
        glDeleteShader(_idFS);
        _idFS = 0;
    }

    // Delete the program
    if (_id) {
        _attributes.clear();
        _uniforms.clear();
        glDeleteProgram(_id);
        _id = 0;
        _isLinked = false;
    }
}

bool vg::GLProgram::addShader(const ShaderSource& data) {
    // Check current state
    if (getIsLinked() || !getIsCreated()) {
        onShaderCompilationError("Cannot add a shader to a fully created or non-existent program");
        return false;
    }

    // Check for preexisting stages
    switch (data.stage) {
        case ShaderType::VERTEX_SHADER:
            if (_idVS != 0) {
                onShaderCompilationError("Attempting to add another vertex shader");
                return false;
            }
            break;
        case ShaderType::FRAGMENT_SHADER:
            if (_idFS != 0) {
                onShaderCompilationError("Attempting to add another fragment shader");
                return false;
            }
            break;
        default:
            onShaderCompilationError("Shader stage is not supported");
            return false;
    }

    // List of shader code
    const cString* sources = new const cString[data.sources.size() + 1];

    // Version information
    char bufVersion[32];
    sprintf(bufVersion, "#version %d%d%d\n\0", data.version.major, data.version.minor, data.version.revision);
    sources[0] = bufVersion;

    // Append rest of shader code
    for (size_t i = 0; i < data.sources.size(); i++) {
        sources[i + 1] = data.sources[i];
    }

    // Compile shader
    VGShader idS = glCreateShader((VGEnum)data.stage);
    glShaderSource(idS, data.sources.size() + 1, sources, 0);
    glCompileShader(idS);
    delete[] sources;

    // Check status
    i32 status;
    glGetShaderiv(idS, GL_COMPILE_STATUS, &status);
    if (status != 1) {
        int infoLogLength;
        glGetShaderiv(idS, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::vector<char> FragmentShaderErrorMessage(infoLogLength);
        glGetShaderInfoLog(idS, infoLogLength, NULL, FragmentShaderErrorMessage.data());
        onShaderCompilationError(&FragmentShaderErrorMessage[0]);
        glDeleteShader(idS);
        return false;
    }

    // Add shader to stage
    switch (data.stage) {
        case ShaderType::VERTEX_SHADER:
            _idVS = idS;
            break;
        case ShaderType::FRAGMENT_SHADER:
            _idFS = idS;
            break;
    }
    return true;
}
bool vg::GLProgram::addShader(const ShaderType& type, const cString code, const ShaderLanguageVersion& version /*= DEFAULT_SHADING_LANGUAGE_VERSION*/) {
    ShaderSource src;
    src.stage = type;
    src.sources.push_back(code);
    src.version = version;
    return addShader(src);
}

void vg::GLProgram::setAttribute(nString name, VGAttribute index) {
    // Adding attributes to a linked program does nothing
    if (getIsLinked() || !getIsCreated()) return;

    // Set the custom attribute
    glBindAttribLocation(_id, index, name.c_str());
    _attributes[name] = index;
}
void vg::GLProgram::setAttributes(const std::map<nString, VGAttribute>& attr) {
    // Adding attributes to a linked program does nothing
    if (getIsLinked() || !getIsCreated()) return;

    // Set the custom attributes
    for (auto& binding : attr) {
        glBindAttribLocation(_id, binding.second, binding.first.c_str());
        _attributes[binding.first] = binding.second;
    }
}
void vg::GLProgram::setAttributes(const std::vector<AttributeBinding>& attr) {
    // Adding attributes to a linked program does nothing
    if (getIsLinked() || !getIsCreated()) return;

    // Set the custom attributes
    for (auto& binding : attr) {
        glBindAttribLocation(_id, binding.second, binding.first.c_str());
        _attributes[binding.first] = binding.second;
    }
}
void vg::GLProgram::setAttributes(const std::vector<nString>& attr) {
    // Adding attributes to a linked program does nothing
    if (getIsLinked() || !getIsCreated()) return;

    // Set the custom attributes
    for (ui32 i = 0; i < attr.size(); i++) {
        glBindAttribLocation(_id, i, attr[i].c_str());
        _attributes[attr[i]] = i;
    }
}

bool vg::GLProgram::link() {
    // Check internal state
    if (getIsLinked() || !getIsCreated()) {
        onProgramLinkError("Cannot link a fully created or non-existent program");
        return false;
    }

    // Check for available shaders
    if (!_idVS || !_idFS) {
        onProgramLinkError("Insufficient stages for a program link");
        return false;
    }

    // Link The Program
    glAttachShader(_id, _idVS);
    glAttachShader(_id, _idFS);
    glLinkProgram(_id);

    // Detach and delete shaders
    glDetachShader(_id, _idVS);
    glDetachShader(_id, _idFS);
    glDeleteShader(_idVS);
    glDeleteShader(_idFS);
    _idVS = 0;
    _idFS = 0;

    // Check the link status
    i32 status;
    glGetProgramiv(_id, GL_LINK_STATUS, &status);
    _isLinked = status == 1;
    if (!_isLinked) {
        onProgramLinkError("Program had link errors");
        return false;
    }
    return true;
}

void vg::GLProgram::initAttributes() {
    if (!getIsLinked()) return;

    // Obtain attribute count
    i32 count;
    glGetProgramiv(_id, GL_ACTIVE_ATTRIBUTES, &count);

    // Necessary info
    char name[PROGRAM_VARIABLE_MAX_LENGTH + 1];
    i32 len;
    GLenum type;
    i32 amount;

    // Enumerate through attributes
    for (int i = 0; i < count; i++) {
        // Get attribute info
        glGetActiveAttrib(_id, i, PROGRAM_VARIABLE_MAX_LENGTH, &len, &amount, &type, name);
        name[len] = 0;
        VGAttribute loc = glGetAttribLocation(_id, name);

        // Get rid of system attributes
        if (strncmp(name, PROGRAM_VARIABLE_IGNORED_PREFIX, PROGRAM_VARIABLE_IGNORED_PREFIX_LEN) != 0 && loc != -1) {
            _attributes[name] = loc;
        }
    }
}
void vg::GLProgram::initUniforms() {
    if (!getIsLinked()) return;

    // Obtain uniform count
    i32 count;
    glGetProgramiv(_id, GL_ACTIVE_UNIFORMS, &count);

    // Necessary info
    char name[PROGRAM_VARIABLE_MAX_LENGTH + 1];
    i32 len;
    GLenum type;
    i32 amount;

    // Enumerate through uniforms
    for (int i = 0; i < count; i++) {
        // Get uniform info
        glGetActiveUniform(_id, i, PROGRAM_VARIABLE_MAX_LENGTH, &len, &amount, &type, name);
        name[len] = 0;
        VGUniform loc = glGetUniformLocation(_id, name);

        // Get rid of system uniforms
        if (strncmp(name, PROGRAM_VARIABLE_IGNORED_PREFIX, PROGRAM_VARIABLE_IGNORED_PREFIX_LEN) != 0 && loc != -1) {
            _uniforms[name] = loc;
        }
    }
}

void vg::GLProgram::enableVertexAttribArrays() const {
    for (auto& attrBind : _attributes) {
        glEnableVertexAttribArray(attrBind.second);
    }
}
void vg::GLProgram::disableVertexAttribArrays() const {
    for (auto& attrBind : _attributes) {
        glDisableVertexAttribArray(attrBind.second);
    }
}

void vg::GLProgram::use() {
    if (!getIsInUse()) {
        _programInUse = this;
        glUseProgram(_id);
    }
}
void vg::GLProgram::unuse() {
    if (_programInUse) {
        _programInUse = nullptr;
        glUseProgram(0);
    }
}


