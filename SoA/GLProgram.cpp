#include "stdafx.h"
#include "GLProgram.h"

#include "IOManager.h"

// We Don't Want To Get OpenGL's Default Attributes
#define PROGRAM_VARIABLE_IGNORED_PREFIX "gl_"
#define PROGRAM_VARIABLE_IGNORED_PREFIX_LEN 3
// Will Our Variable Names Honestly Be More Than A KB
#define PROGRAM_VARIABLE_MAX_LENGTH 1024

GLProgram::GLProgram(bool init /*= false*/) :
_id(0),
_idVS(0),
_idFS(0),
_isLinked(false) {
    if (init) this->init();
}

void GLProgram::init() {
    if (getIsCreated()) return;
    _isLinked = false;
    _id = glCreateProgram();
}
void GLProgram::destroy() {
    // Delete The Shaders
    if (_idVS) {
        if (_id) glDetachShader(_id, _idVS);
        glDeleteShader(_idVS);
        _idVS = 0;
    }
    if (_idFS) {
        if (_id) glDetachShader(_id, _idFS);
        glDeleteShader(_idFS);
        _idFS = 0;
    }

    // Delete The Program
    if (_id) {
        glDeleteProgram(_id);
        _id = 0;
    }
}

void GLProgram::addShader(i32 type, const cString src) {
    // Check Current State
    if (getIsLinked() || !getIsCreated()) return;
    switch (type) {
    case GL_VERTEX_SHADER:
        if (_idVS != 0) {
            printf("Attempting To Add Another Vertex Shader To Program\n");
            throw 2;
        }
        break;
    case GL_FRAGMENT_SHADER:
        if (_idFS != 0) {
            printf("Attempting To Add Another Fragment Shader To Program\n");
            throw 2;
        }
        break;
    default:
        printf("Shader Type Is Not Supported\n");
        throw 2;
    }

    // Compile The Shader
    ui32 idS = glCreateShader(type);
    glShaderSource(idS, 1, &src, 0);
    glCompileShader(idS);

    // Check Status
    i32 status;
    glGetShaderiv(idS, GL_COMPILE_STATUS, &status);
    if (status != 1) {
        glDeleteShader(idS);
        printf("Shader Had Compilation Errors\n");
        throw 2;
    }

    // Bind Shader To Program
    glAttachShader(_id, idS);
    switch (type) {
    case GL_VERTEX_SHADER: _idVS = idS; break;
    case GL_FRAGMENT_SHADER: _idFS = idS; break;
    }
}
void GLProgram::addShaderFile(i32 type, const cString file) {
    IOManager iom;
    const cString src = iom.readFileToString(file);
    addShader(type, src);
    delete [] src;
}

void GLProgram::setAttributes(const std::map<nString, ui32>& attr) {
    // Don't Add Attributes If The Program Is Already Linked
    if (_isLinked) return;

    // Set The Custom Attributes
    auto attrIter = attr.begin();
    while (attrIter != attr.end()) {
        glBindAttribLocation(_id, attrIter->second, attrIter->first.c_str());
        _attributes[attrIter->first] = attrIter->second;
        attrIter++;
    }
}
bool GLProgram::link() {
    // Don't Relink Or Attempt A Non-initialized Link
    if (getIsLinked() || !getIsCreated()) return false;

    // Link The Program
    glLinkProgram(_id);
    glValidateProgram(_id);

    // Get The Link Status
    i32 status;
    glGetProgramiv(_id, GL_LINK_STATUS, &status);
    _isLinked = status == 1;
    return _isLinked;
}

void GLProgram::initAttributes() {
    // How Many Attributes Are In The Program
    i32 count;
    glGetProgramiv(_id, GL_ACTIVE_ATTRIBUTES, &count);

    // Necessary Info
    char name[PROGRAM_VARIABLE_MAX_LENGTH + 1];
    i32 len;

    // Attribute Type
    GLenum type;
    i32 amount;

    // Enumerate Through Attributes
    for (int i = 0; i < count; i++) {
        // Get Attribute Info
        glGetActiveAttrib(_id, i, PROGRAM_VARIABLE_MAX_LENGTH, &len, &amount, &type, name);
        name[len] = 0;
        ui32 loc = glGetAttribLocation(_id, name);

        // Get Rid Of System Attributes
        if (strncmp(name, PROGRAM_VARIABLE_IGNORED_PREFIX, PROGRAM_VARIABLE_IGNORED_PREFIX_LEN) != 0 && loc != -1) {
            _attributes[name] = loc;
        }
    }
}
void GLProgram::initUniforms() {
    // How Many Uniforms Are In The Program
    i32 count;
    glGetProgramiv(_id, GL_ACTIVE_UNIFORMS, &count);

    // Necessary Info
    char name[PROGRAM_VARIABLE_MAX_LENGTH + 1];
    i32 len;

    // Uniform Type
    GLenum type;
    i32 amount;

    // Enumerate Through Uniforms
    for (int i = 0; i < count; i++) {
        // Get Uniform Info
        glGetActiveUniform(_id, i, PROGRAM_VARIABLE_MAX_LENGTH, &len, &amount, &type, name);
        name[len] = 0;
        ui32 loc = glGetUniformLocation(_id, name);

        // Get Rid Of System Uniforms
        if (strncmp(name, PROGRAM_VARIABLE_IGNORED_PREFIX, PROGRAM_VARIABLE_IGNORED_PREFIX_LEN) != 0 && loc != -1) {
            _uniforms[name] = loc;
        }
    }
}

void GLProgram::use() {
    if (_programInUse != this) {
        _programInUse = this;
        glUseProgram(_id);
    }
}
void GLProgram::unuse() {
    if (_programInUse) {
        _programInUse = nullptr;
        glUseProgram(0);
    }
}

GLProgram* GLProgram::_programInUse = nullptr;
