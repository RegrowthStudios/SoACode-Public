#pragma once

// Wraper class for shader types
enum class ShaderType {
    VERTEX = GL_VERTEX_SHADER,
    FRAGMENT = GL_FRAGMENT_SHADER
};

// Encapsulates A Simple OpenGL Program And Its Shaders
class GLProgram {
public:
    // Create And Possibly Initialize Program
    GLProgram(bool init = false);

    // Create And Free GPU Resources
    void init();
    void destroy();
    bool getIsCreated() const {
        return _id != 0;
    }
    const int& getID() const  {
        return _id;
    }

    // Attach Program Build Information
    void addShader(ShaderType type, const cString src);
    void addShaderFile(ShaderType type, const cString file);

    // Build The Program
    void setAttribute(nString name, ui32 index);
    void setAttributes(const std::map<nString, ui32>& attr);
    void setAttributes(const std::vector<std::pair<nString, ui32> >& attr);
    bool link();
    const bool& getIsLinked() const {
        return _isLinked;
    }

    // Create Mappings For Program Variables
    void initAttributes();
    void initUniforms();

    // Unmap Program Variables
    const ui32& getAttribute(const nString& name) const {
        return _attributes.at(name);
    }
    const ui32& getUniform(const nString& name) const {
        return _uniforms.at(name);
    }

    // Program Setup For The GPU
    void use();
    static void unuse();
    const bool& getIsInUse() const {
        return _programInUse == this;
    }

    static GLProgram* getCurrentProgram() {
        return _programInUse;
    }
private:
    // The Current Program In Use
    static GLProgram* _programInUse;

    // Program ID
    ui32 _id;

    // Shader IDs
    ui32 _idVS, _idFS;

    // True On A Successful Link
    bool _isLinked;

    // Attribute And Uniform Map
    std::map<nString, ui32> _attributes;
    std::map<nString, ui32> _uniforms;
};