#include "stdafx.h"
#include "ShaderAssetLoader.h"

#include <tuple>

#include <Vorb/io/IOManager.h>
#include <Vorb/io/Keg.h>

// TODO(Ben): Use shader loader
namespace {
    typedef std::pair<nString, VGAttribute> AttributeBinding;
    KEG_TYPE_DECL(Anon_AttributeBinding);
    KEG_TYPE_DEF(Anon_AttributeBinding, AttributeBinding, kt) {
        using namespace keg;
        kt.addValue("Name", Value::value(&AttributeBinding::first));
        kt.addValue("Location", Value::value(&AttributeBinding::second));
    }

    struct ShaderAssetInformation {
    public:
        nString vertexFile;
        nString fragmentFile;
        Array<AttributeBinding> binds;
        Array<AttributeBinding> frags;
        Array<nString> defines;
    };
    KEG_TYPE_DECL(Anon_ShaderAssetInformation);
    KEG_TYPE_DEF(Anon_ShaderAssetInformation, ShaderAssetInformation, kt) {
        using namespace keg;
        kt.addValue("Vertex", Value::value(&ShaderAssetInformation::vertexFile));
        kt.addValue("Fragment", Value::value(&ShaderAssetInformation::fragmentFile));
        kt.addValue("Attributes", Value::array(offsetof(ShaderAssetInformation, binds), Value::custom(0, "Anon_AttributeBinding")));
        kt.addValue("Fragments", Value::array(offsetof(ShaderAssetInformation, frags), Value::custom(0, "Anon_AttributeBinding")));
        kt.addValue("Defines", Value::array(offsetof(ShaderAssetInformation, defines), BasicType::STRING));
    }

    void initProgram(Sender, void* ptr) {
        auto& program = ((ShaderAsset*)ptr)->program;
        program.init();
    }
    void finishProgram(Sender, void* ptr) {
        auto& program = ((ShaderAsset*)ptr)->program;
        program.link();
        program.initAttributes();
        program.initUniforms();
    }

    typedef std::tuple<ShaderAsset*, vg::ShaderType, const cString, Array<nString>&> CompileArgs;
    void compileShader(Sender, void* ptr) {
        auto& args = *(CompileArgs*)ptr;

        vg::ShaderSource source;
        source.stage = std::get<1>(args);

        // Add macros
        Array<nString>& defines = std::get<3>(args);
        char buf[1024];
        char* dest = buf;
        for (size_t i = 0; i < defines.size(); i++) {
            source.sources.push_back(dest);
            dest += sprintf(dest, "#define %s\n", defines[i].c_str());
        }

        // Add shader code
        source.sources.push_back(std::get<2>(args));

        // Compile
        std::get<0>(args)->program.addShader(source);
    }

    typedef std::tuple<ShaderAsset*, ShaderAssetInformation*> BindArgs;
    void bindAttributes(Sender, void* ptr) {
        auto& args = *(BindArgs*)ptr;
        auto& binds = std::get<1>(args)->binds;
        vg::GLProgram& program = std::get<0>(args)->program;
        for (size_t i = 0; i < binds.size(); i++) {
            program.setAttribute(binds[i].first, binds[i].second);
        }
    }
    void bindFragments(Sender, void* ptr) {
        auto& args = *(BindArgs*)ptr;
        auto& binds = std::get<1>(args)->binds;
        vg::GLProgram& program = std::get<0>(args)->program;
        for (size_t i = 0; i < binds.size(); i++) {
            program.bindFragDataLocation(binds[i].second, binds[i].first.c_str());
        }
    }
}

void vcore::AssetBuilder<ShaderAsset>::create(const vpath& p, OUT ShaderAsset* asset, vcore::RPCManager& rpc) {
    GLRPC so(asset);
    vio::IOManager iom;

    // Read the YAML information
    ShaderAssetInformation info{};
    const cString str = iom.readFileToString(p);
    keg::parse(&info, str, "Anon_ShaderAssetInformation");
    delete[] str;

    // Set the root directory to the YAML info file
    vpath root = p;
    root--;
    iom.setSearchDirectory(root);

    // Initialize the program
    so.set([](Sender s, void* d) { initProgram(s, d); });
    rpc.invoke(&so);

    auto func1=[=](Sender, const nString& msg) {
        printf("PROG COMP ERROR:\n%s\n", msg.c_str());
    };
    auto d1 = makeDelegate(&func1);

    auto func2=[=](Sender, const nString& msg) {
        printf("PROG LINK ERROR:\n%s\n", msg.c_str());
    };
    auto d2 = makeDelegate(&func2);

    asset->program.onShaderCompilationError += d1;
    asset->program.onProgramLinkError += d2;

    { // Read vertex file
        str = iom.readFileToString(info.vertexFile);
        CompileArgs args(asset, vg::ShaderType::VERTEX_SHADER, str, info.defines);
        so.data.userData = &args;
        so.set([](Sender s, void* d) { compileShader(s, d); });
        rpc.invoke(&so);
        delete[] str;
    }

    { // Read fragment file
        str = iom.readFileToString(info.fragmentFile);
        CompileArgs args(asset, vg::ShaderType::FRAGMENT_SHADER, str, info.defines);
        so.data.userData = &args;
        so.set([](Sender s, void* d) { compileShader(s, d); });
        rpc.invoke(&so);
        delete[] str;
    }

    { // Bind all attribute locations
        BindArgs args(asset, &info);
        so.data.userData = &args;
        so.set([](Sender s, void* d) { bindAttributes(s, d); });
        rpc.invoke(&so);
    }

    if (info.frags.size() > 0) {
        // Bind all fragment locations
        BindArgs args(asset, &info);
        so.data.userData = &args;
        so.set([](Sender s, void* d) { bindFragments(s, d); });
        rpc.invoke(&so);
    }

    // Link the program
    so.data.userData = asset;
    so.set([](Sender s, void* d) { finishProgram(s, d); });
    rpc.invoke(&so);

    asset->program.onShaderCompilationError -= d1;
    asset->program.onProgramLinkError -= d2;
}
void vcore::AssetBuilder<ShaderAsset>::destroy(ShaderAsset* asset) {
    asset->program.dispose();
}
