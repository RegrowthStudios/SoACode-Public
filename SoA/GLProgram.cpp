#include "stdafx.h"
#include "GLProgram.h"

#include "IOManager.h"

// We Don't Want To Get OpenGL's Default Attributes
#define PROGRAM_VARIABLE_IGNORED_PREFIX "gl_"
#define PROGRAM_VARIABLE_IGNORED_PREFIX_LEN 3
// Will Our Variable Names Honestly Be More Than A KB
#define PROGRAM_VARIABLE_MAX_LENGTH 1024

vg::GLProgram* vg::GLProgram::_programInUse = nullptr;

vg::GLProgram::GLProgram(bool init /*= false*/) :
    _id(0),
    _idVS(0),
    _idFS(0),
    _isLinked(false) {
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

bool vg::GLProgram::addShader(ShaderType type, const cString src, const cString defines /*= nullptr*/) {
    // Get the GLenum shader type from the wrapper
    i32 glType = static_cast<GLenum>(type);
    // Check Current State
    if (getIsLinked() || !getIsCreated()) return false;
    switch (glType) {
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
    const cString sources[3];
    ui32 idS = glCreateShader(glType);
    char bufVersion[32];
    sprintf(bufVersion, "#version %d%d%d\n\0", _versionMajor, _versionMinor, _versionRevision);
    sources[0] = bufVersion;
    if (defines) {
        sources[1] = defines;
        sources[2] = src;
        glShaderSource(idS, 3, sources, 0);
    } else {
        sources[1] = src;
        glShaderSource(idS, 2, sources, 0);
    }
    glCompileShader(idS);

    // Check Status
    i32 status;
    glGetShaderiv(idS, GL_COMPILE_STATUS, &status);
    if (status != 1) {
        int infoLogLength;
        printf("Shader Had Compilation Errors\n");
        glGetShaderiv(idS, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::vector<char> FragmentShaderErrorMessage(infoLogLength);
        glGetShaderInfoLog(idS, infoLogLength, NULL, FragmentShaderErrorMessage.data());
        fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);
        glDeleteShader(idS);
        return false;
    }

    // Bind Shader To Program
    glAttachShader(_id, idS);
    switch (glType) {
        case GL_VERTEX_SHADER: _idVS = idS; break;
        case GL_FRAGMENT_SHADER: _idFS = idS; break;
    }
    return true;
}

bool vg::GLProgram::addShaderFile(ShaderType type, const cString file, const cString defines /*= nullptr*/) {
    IOManager iom;
    nString src;
    iom.readFileToString(file, src);

    return addShader(type, src.c_str(), defines);
}

void vg::GLProgram::setAttribute(nString name, ui32 index) {
    // Don't Add Attributes If The Program Is Already Linked
    if (_isLinked) return;

    glBindAttribLocation(_id, index, name.c_str());
    _attributes[name] = index;
}

void vg::GLProgram::setAttributes(const std::map<nString, ui32>& attr) {
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

void vg::GLProgram::setAttributes(const std::vector<std::pair<nString, ui32> >& attr) {
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

void vg::GLProgram::setAttributes(const std::vector<nString>& attr) {
    // Don't Add Attributes If The Program Is Already Linked
    if (_isLinked) return;

    // Set The Custom Attributes
    for (ui32 i = 0; i < attr.size(); i++) {
        glBindAttribLocation(_id, i, attr[i].c_str());
        _attributes[attr[i]] = i;
    }
}

vg::GLProgram& vg::GLProgram::setVersion(ui32 major, ui32 minor, ui32 revision) {
    if (getIsCreated()) return *this;
    _versionMajor = major;
    _versionMinor = minor;
    _versionRevision = revision;
    return *this;
}

bool vg::GLProgram::link() {
    // Don't Relink Or Attempt A Non-initialized Link
    if (getIsLinked() || !getIsCreated()) return false;

    // Link The Program
    glLinkProgram(_id);

    // Don't need the shaders anymore
    if (_idVS) {
        glDeleteShader(_idVS);
        _idVS = 0;
    }
    if (_idFS) {
        glDeleteShader(_idFS);
        _idFS = 0;
    }

    glValidateProgram(_id);

    // Get The Link Status
    i32 status;
    glGetProgramiv(_id, GL_LINK_STATUS, &status);
    _isLinked = status == 1;
    return _isLinked;
}

void vg::GLProgram::initAttributes() {
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
void vg::GLProgram::initUniforms() {
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

void vg::GLProgram::enableVertexAttribArrays() const {
    for (auto it = _attributes.begin(); it != _attributes.end(); it++) {
        glEnableVertexAttribArray(it->second);
    }
}

void vg::GLProgram::disableVertexAttribArrays() const {
    for (auto it = _attributes.begin(); it != _attributes.end(); it++) {
        glDisableVertexAttribArray(it->second);
    }
}

void vg::GLProgram::use() {
    if (_programInUse != this) {
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

