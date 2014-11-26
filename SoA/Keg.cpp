#include "stdafx.h"
#include "Keg.h"


#pragma region YAML Conversion Functions For GLM Types And cStrings
namespace YAML {
    template<>
    struct convert<cString> {
        static Node encode(const cString& rhs) {
            Node node;
            node.push_back(nString(rhs));
            return node;
        }
        static bool decode(const Node& node, cString& rhs) {
            nString s;
            bool success = convert<nString>::decode(node, s);
            if (success) {
                size_t len = s.length();
                rhs = new char[len + 1];
                memcpy(rhs, &s[0], len + 1);
            }
            return success;
        }
    };
    template<>
    struct convert<i8> {
        static Node encode(const i8& rhs) {
            Node node;
            node.push_back(rhs);
            return node;
        }
        static bool decode(const Node& node, i8& rhs) {
            ui16 v;
            bool success = convert<ui16>::decode(node, v);
            if (success) rhs = static_cast<i8>(v);
            return success;
        }
    };
    template<typename T, typename T_Comp, i32 C>
    struct convertVec {
        static Node encode(const T& rhs) {
            Node node;
            for (i32 i = 0; i < C; i++) node.push_back(rhs[i]);
            return node;
        }
        static bool decode(const Node& node, T& rhs) {
            if (!node.IsSequence() || node.size() != C) return false;
            for (i32 i = 0; i < C; i++) rhs[i] = node[i].as<T_Comp>();
            return true;
        }
    };
    template<typename T, typename T_Comp, i32 C>
    struct convertVecI8 {
        static Node encode(const T& rhs) {
            Node node;
            for (i32 i = 0; i < C; i++) node.push_back(rhs[i]);
            return node;
        }
        static bool decode(const Node& node, T& rhs) {
            if (!node.IsSequence() || node.size() != C) return false;
            for (i32 i = 0; i < C; i++) rhs[i] = node[i].as<T_Comp>();
            return true;
        }
    };
#define YAML_EMITTER_VEC(T, C) \
    YAML::Emitter& operator << (YAML::Emitter& out, const T& v) MACRO_PARAN_L \
    out << YAML::Flow; \
    out << YAML::BeginSeq; \
    for (i32 i = 0; i < C; i++) out << v[i]; \
    out << YAML::EndSeq; \
    return out; \
    MACRO_PARAN_R

#define KEG_DECL_CONV_VEC_COMP(TYPE, TC, COUNT) YAML_EMITTER_VEC(TYPE##v##COUNT, COUNT) template<> struct convert<TYPE##v##COUNT> : public convertVec<TYPE##v##COUNT, TC, COUNT> {}
#define KEG_DECL_CONV_VEC(TYPE, COUNT) KEG_DECL_CONV_VEC_COMP(TYPE, TYPE, COUNT)
    KEG_DECL_CONV_VEC_COMP(i8, i16, 2);
    KEG_DECL_CONV_VEC_COMP(i8, i16, 3);
    KEG_DECL_CONV_VEC_COMP(i8, i16, 4);
    KEG_DECL_CONV_VEC(i16, 2);
    KEG_DECL_CONV_VEC(i16, 3);
    KEG_DECL_CONV_VEC(i16, 4);
    KEG_DECL_CONV_VEC(i32, 2);
    KEG_DECL_CONV_VEC(i32, 3);
    KEG_DECL_CONV_VEC(i32, 4);
    KEG_DECL_CONV_VEC(i64, 2);
    KEG_DECL_CONV_VEC(i64, 3);
    KEG_DECL_CONV_VEC(i64, 4);
    KEG_DECL_CONV_VEC_COMP(ui8, ui16, 2);
    KEG_DECL_CONV_VEC_COMP(ui8, ui16, 3);
    KEG_DECL_CONV_VEC_COMP(ui8, ui16, 4);
    KEG_DECL_CONV_VEC(ui16, 2);
    KEG_DECL_CONV_VEC(ui16, 3);
    KEG_DECL_CONV_VEC(ui16, 4);
    KEG_DECL_CONV_VEC(ui32, 2);
    KEG_DECL_CONV_VEC(ui32, 3);
    KEG_DECL_CONV_VEC(ui32, 4);
    KEG_DECL_CONV_VEC(ui64, 2);
    KEG_DECL_CONV_VEC(ui64, 3);
    KEG_DECL_CONV_VEC(ui64, 4);
    KEG_DECL_CONV_VEC(f32, 2);
    KEG_DECL_CONV_VEC(f32, 3);
    KEG_DECL_CONV_VEC(f32, 4);
    KEG_DECL_CONV_VEC(f64, 2);
    KEG_DECL_CONV_VEC(f64, 3);
    KEG_DECL_CONV_VEC(f64, 4);
}
#pragma endregion

namespace Keg {
#define KEG_BASIC_NUM_MAP(TYPE) \
    { BasicType::TYPE, #TYPE }, \
    {BasicType::TYPE##_V2, #TYPE "_V2"}, \
    {BasicType::TYPE##_V3, #TYPE "_V3"}, \
    {BasicType::TYPE##_V4, #TYPE "_V4"}
    std::map<BasicType, nString> basicTypes = {
        KEG_BASIC_NUM_MAP(I8),
        KEG_BASIC_NUM_MAP(I16),
        KEG_BASIC_NUM_MAP(I32),
        KEG_BASIC_NUM_MAP(I64),
        KEG_BASIC_NUM_MAP(UI8),
        KEG_BASIC_NUM_MAP(UI16),
        KEG_BASIC_NUM_MAP(UI32),
        KEG_BASIC_NUM_MAP(UI64),
        KEG_BASIC_NUM_MAP(F32),
        KEG_BASIC_NUM_MAP(F64),
        { BasicType::BOOL, "Bool" },
        { BasicType::STRING, "String" },
        { BasicType::C_STRING, "CString" }
    };

    Value Value::basic(BasicType t, i32 off) {
        Value kv = {};
        kv.type = t;
        kv.offset = off;
        kv.typeName.clear();
        return kv;
    }
    Value Value::custom(nString t, i32 off, bool isEnum /*= false*/) {
        Value kv = {};
        kv.type = isEnum ? BasicType::ENUM : BasicType::CUSTOM;
        kv.offset = off;
        kv.typeName = t;
        return kv;
    }
    Value Value::array(i32 off, const Value& interior) {
        Value kv = {};
        kv.type = BasicType::ARRAY;
        kv.offset = off;
        kv.typeName.clear();
        kv.interiorValue = new Value(interior);
        return kv;
    }
    Value Value::array(i32 off, const BasicType& t) {
        return array(off, Value::basic(t, 0));
    }
    Value Value::ptr(i32 off, const Value& interior) {
        Value kv = {};
        kv.type = BasicType::PTR;
        kv.offset = off;
        kv.typeName.clear();
        kv.interiorValue = new Value(interior);
        return kv;
    }
    Value Value::ptr(i32 off, const BasicType& t) {
        return ptr(off, Value::basic(t, 0));
    }

    Type::Type() : _values(), _sizeInBytes(0) {}
    Type::Type(size_t sizeInBytes, const nString& name, Environment* env) :
        _values(),
        _sizeInBytes(sizeInBytes) {
        if (env) env->addType(name, this);
        getGlobalEnvironment()->addType(name, this);
    }

    void Type::addValue(const nString& name, const Value& type) {
        _values[name] = type;
    }
    void Type::addSuper(const Type* type, size_t offset /*= 0*/) {
        for (auto kvPair : type->_values) {
            kvPair.second.offset += offset;
            _values[kvPair.first] = kvPair.second;
        }
    }

    Enum::Enum(size_t sizeInBytes, const nString& name, Environment* env) :
        _sizeInBytes(sizeInBytes) {
        switch (_sizeInBytes) {
        case 1: _fSetter = setValue8; _fGetter = getValue8; break;
        case 2: _fSetter = setValue16; _fGetter = getValue16; break;
        case 4: _fSetter = setValue32; _fGetter = getValue32; break;
        default: _fSetter = setValue64; _fGetter = getValue64; break;
        }
        if (env) env->addEnum(name, this);
        getGlobalEnvironment()->addEnum(name, this);
    }

    void Enum::setValue64(void* data, const nString& s, Enum* e) {
        auto kv = e->_values.find(s);
        if (kv != e->_values.end()) *((ui64*)data) = (ui64)kv->second;
    }
    void Enum::setValue32(void* data, const nString& s, Enum* e) {
        auto kv = e->_values.find(s);
        if (kv != e->_values.end()) *((ui32*)data) = (ui32)kv->second;
    }
    void Enum::setValue16(void* data, const nString& s, Enum* e) {
        auto kv = e->_values.find(s);
        if (kv != e->_values.end()) *((ui16*)data) = (ui16)kv->second;
    }
    void Enum::setValue8(void* data, const nString& s, Enum* e) {
        auto kv = e->_values.find(s);
        if (kv != e->_values.end()) *((ui8*)data) = (ui8)kv->second;
    }
    nString Enum::getValue64(const void* data, Enum* e) {
        ui64 key = *((ui64*)data);
        auto kv = e->_valuesRev.find(key);
        if (kv != e->_valuesRev.end()) return kv->second;
        return "";
    }
    nString Enum::getValue32(const void* data, Enum* e) {
        ui32 key = *((ui32*)data);
        auto kv = e->_valuesRev.find(key);
        if (kv != e->_valuesRev.end()) return kv->second;
        return "";
    }
    nString Enum::getValue16(const void* data, Enum* e) {
        ui16 key = *((ui16*)data);
        auto kv = e->_valuesRev.find(key);
        if (kv != e->_valuesRev.end()) return kv->second;
        return "";
    }
    nString Enum::getValue8(const void* data, Enum* e) {
        ui8 key = *((ui8*)data);
        auto kv = e->_valuesRev.find(key);
        if (kv != e->_valuesRev.end()) return kv->second;
        return "";
    }

    Environment::Environment() :
        _uuid(KEG_BAD_TYPE_ID) {
        Value kv;
        Type kt;

#define NUM_TYPE(TYPE, C_TYPE, NUM) \
    _internalTypes[(i32)BasicType::TYPE]._sizeInBytes = sizeof(C_TYPE); \
    kv = Value::basic(BasicType::TYPE, sizeof(C_TYPE)* 0); \
    _internalTypes[(i32)BasicType::TYPE].addValue("value0", kv); \
    _internalTypes[(i32)BasicType::TYPE].addValue("x", kv); \
    _internalTypes[(i32)BasicType::TYPE].addValue("r", kv); \
    _internalTypes[(i32)BasicType::TYPE].addValue("s", kv); \
    _internalTypes[(i32)BasicType::TYPE].addValue("0", kv); \
    addType(#TYPE, &_internalTypes[(i32)BasicType::TYPE]); \
    _internalTypes[(i32)BasicType::TYPE##_V2].addSuper(&_internalTypes[(i32)BasicType::TYPE]); \
    _internalTypes[(i32)BasicType::TYPE##_V2]._sizeInBytes = sizeof(C_TYPE)* 2; \
    kv = Value::basic(BasicType::TYPE, sizeof(C_TYPE)* 1); \
    _internalTypes[(i32)BasicType::TYPE##_V2].addValue("value1", kv); \
    _internalTypes[(i32)BasicType::TYPE##_V2].addValue("y", kv); \
    _internalTypes[(i32)BasicType::TYPE##_V2].addValue("g", kv); \
    _internalTypes[(i32)BasicType::TYPE##_V2].addValue("t", kv); \
    _internalTypes[(i32)BasicType::TYPE##_V2].addValue("1", kv); \
    addType(#TYPE"_V2", &_internalTypes[(i32)BasicType::TYPE##_V2]); \
    _internalTypes[(i32)BasicType::TYPE##_V3].addSuper(&_internalTypes[(i32)BasicType::TYPE##_V2]); \
    _internalTypes[(i32)BasicType::TYPE##_V3]._sizeInBytes = sizeof(C_TYPE)* 3; \
    kv = Value::basic(BasicType::TYPE, sizeof(C_TYPE)* 2); \
    _internalTypes[(i32)BasicType::TYPE##_V3].addValue("value2", kv); \
    _internalTypes[(i32)BasicType::TYPE##_V3].addValue("z", kv); \
    _internalTypes[(i32)BasicType::TYPE##_V3].addValue("b", kv); \
    _internalTypes[(i32)BasicType::TYPE##_V3].addValue("p", kv); \
    _internalTypes[(i32)BasicType::TYPE##_V3].addValue("2", kv); \
    addType(#TYPE"_V3", &_internalTypes[(i32)BasicType::TYPE##_V3]); \
    _internalTypes[(i32)BasicType::TYPE##_V4].addSuper(&_internalTypes[(i32)BasicType::TYPE##_V3]); \
    _internalTypes[(i32)BasicType::TYPE##_V4]._sizeInBytes = sizeof(C_TYPE)* 4; \
    kv = Value::basic(BasicType::TYPE, sizeof(C_TYPE)* 3); \
    _internalTypes[(i32)BasicType::TYPE##_V4].addValue("value3", kv); \
    _internalTypes[(i32)BasicType::TYPE##_V4].addValue("w", kv); \
    _internalTypes[(i32)BasicType::TYPE##_V4].addValue("a", kv); \
    _internalTypes[(i32)BasicType::TYPE##_V4].addValue("q", kv); \
    _internalTypes[(i32)BasicType::TYPE##_V4].addValue("3", kv); \
    addType(#TYPE"_V4", &_internalTypes[(i32)BasicType::TYPE##_V4]); \
    kv = Value::basic(BasicType::TYPE, sizeof(C_TYPE)* 0); \
    _internalTypes[(i32)BasicType::TYPE].addValue("value", kv); \
    kv = Value::basic(BasicType::TYPE##_V2, sizeof(C_TYPE)* 0); \
    _internalTypes[(i32)BasicType::TYPE##_V2].addValue("value", kv); \
    kv = Value::basic(BasicType::TYPE##_V3, sizeof(C_TYPE)* 0); \
    _internalTypes[(i32)BasicType::TYPE##_V3].addValue("value", kv); \
    kv = Value::basic(BasicType::TYPE##_V4, sizeof(C_TYPE)* 0); \
    _internalTypes[(i32)BasicType::TYPE##_V4].addValue("value", kv) \

        NUM_TYPE(I8, i8, 0);
        NUM_TYPE(I16, i16, 4);
        NUM_TYPE(I32, i32, 8);
        NUM_TYPE(I64, i64, 12);
        NUM_TYPE(UI8, ui8, 16);
        NUM_TYPE(UI16, ui16, 20);
        NUM_TYPE(UI32, ui32, 24);
        NUM_TYPE(UI64, ui64, 28);
        NUM_TYPE(F32, f32, 32);
        NUM_TYPE(F64, f64, 36);

        _internalTypes[(i32)BasicType::C_STRING]._sizeInBytes = sizeof(cString);
        kv = Value::basic(BasicType::C_STRING, 0);
        _internalTypes[(i32)BasicType::C_STRING].addValue("value", kv);
        _internalTypes[(i32)BasicType::C_STRING].addValue("string", kv);
        addType("CString", &_internalTypes[(i32)BasicType::C_STRING]);

        _internalTypes[(i32)BasicType::STRING]._sizeInBytes = sizeof(nString);
        kv = Value::basic(BasicType::STRING, 0);
        _internalTypes[(i32)BasicType::STRING].addValue("value", kv);
        _internalTypes[(i32)BasicType::STRING].addValue("string", kv);
        addType("String", &_internalTypes[(i32)BasicType::STRING]);

        _internalTypes[(i32)BasicType::BOOL]._sizeInBytes = sizeof(bool);
        kv = Value::basic(BasicType::BOOL, 0);
        _internalTypes[(i32)BasicType::BOOL].addValue("value", kv);
        _internalTypes[(i32)BasicType::BOOL].addValue("bool", kv);
        addType("Bool", &_internalTypes[(i32)BasicType::BOOL]);

        _internalTypes[(i32)BasicType::ARRAY]._sizeInBytes = sizeof(ArrayBase);
        kv = Value::array(0, Value::custom("", 0, 0));
        _internalTypes[(i32)BasicType::ARRAY].addValue("value", kv);
        _internalTypes[(i32)BasicType::ARRAY].addValue("array", kv);
        _internalTypes[(i32)BasicType::ARRAY].addValue("set", kv);
        _internalTypes[(i32)BasicType::ARRAY].addValue("list", kv);
        _internalTypes[(i32)BasicType::ARRAY].addValue("elements", kv);
        _internalTypes[(i32)BasicType::ARRAY].addValue("data", kv);
        addType("Array", &_internalTypes[(i32)BasicType::ARRAY]);
    }
    ui32 Environment::addType(const nString& name, Type* type) {
        _uuid++;
        _typesByName[name] = type;
        _typesByID[_uuid] = type;
        return _uuid;
    }
    ui32 Environment::addEnum(const nString& name, Enum* type) {
        _uuid++;
        _enumsByName[name] = type;
        _enumsByID[_uuid] = type;
        return _uuid;
    }

    Error parse(ui8* dest, YAML::Node& data, Environment* env, Type* type);
    Error parse(void* dest, const cString data, Type* type, Environment* env /*= nullptr*/) {
        // Test Arguments
        if (env == nullptr) env = getGlobalEnvironment();
        if (dest == nullptr || type == nullptr || data == nullptr) {
            return Error::BAD_ARGUMENT;
        }

        // Parse YAML
        YAML::Node baseNode = YAML::Load(data);
        if (baseNode.IsNull()) return Error::EARLY_EOF;

        // Parse
        return parse((ui8*)dest, baseNode, env, type);
    }
    Error parse(void* dest, const cString data, const nString& typeName, Environment* env /*= nullptr*/) {
        // Test Arguments
        if (env == nullptr) env = getGlobalEnvironment();
        if (dest == nullptr || typeName.empty() || data == nullptr) {
            return Error::BAD_ARGUMENT;
        }

        // Attempt To Find The Type
        Type* type = env->getType(typeName);
        if (type == nullptr) return Error::TYPE_NOT_FOUND;

        // Parse YAML
        YAML::Node baseNode = YAML::Load(data);
        if (baseNode.IsNull()) return Error::EARLY_EOF;

        // Parse
        return parse((ui8*)dest, baseNode, env, type);
    }
    Error parse(void* dest, const cString data, const ui32& typeID, Environment* env /*= nullptr*/) {
        // Test Arguments
        if (env == nullptr) env = getGlobalEnvironment();
        if (dest == nullptr || typeID == KEG_BAD_TYPE_ID || data == nullptr) {
            return Error::BAD_ARGUMENT;
        }

        // Attempt To Find The Type
        Type* type = env->getType(typeID);
        if (type == nullptr) return Error::TYPE_NOT_FOUND;

        // Parse YAML
        YAML::Node baseNode = YAML::Load(data);
        if (baseNode.IsNull()) return Error::EARLY_EOF;

        // Parse
        return parse((ui8*)dest, baseNode, env, type);
    }

    bool write(const ui8* src, YAML::Emitter& e, Environment* env, Type* type);
    nString write(const void* src, Type* type, Environment* env /*= nullptr*/) {
        // Test Arguments
        if (env == nullptr) env = getGlobalEnvironment();
        if (src == nullptr || type == nullptr) {
            return nullptr;
        }

        YAML::Emitter e;
        e << YAML::BeginMap;
        if (!write((ui8*)src, e, env, type)) {
            return nullptr;
        }
        e << YAML::EndMap;
        return nString(e.c_str());
    }
    nString write(const void* src, const nString& typeName, Environment* env /*= nullptr*/) {
        // Test Arguments
        if (env == nullptr) env = getGlobalEnvironment();
        if (src == nullptr || typeName.empty()) {
            return nullptr;
        }

        // Attempt To Find The Type
        Type* type = env->getType(typeName);
        if (type == nullptr) return nullptr;

        YAML::Emitter e;
        e << YAML::BeginMap;
        if (!write((ui8*)src, e, env, type)) {
            return nullptr;
        }
        e << YAML::EndMap;
        return nString(e.c_str());
    }
    nString write(const void* src, const ui32& typeID, Environment* env /*= nullptr*/) {
        // Test Arguments
        if (env == nullptr) env = getGlobalEnvironment();
        if (src == nullptr || typeID == KEG_BAD_TYPE_ID) {
            return nullptr;
        }

        // Attempt To Find The Type
        Type* type = env->getType(typeID);
        if (type == nullptr) return nullptr;

        YAML::Emitter e;
        e << YAML::BeginMap;
        if (!write((ui8*)src, e, env, type)) {
            return nullptr;
        }
        e << YAML::EndMap;
        return nString(e.c_str());
    }

    void evalData(ui8* dest, const Value* decl, YAML::Node& node, Environment* env);

    inline Error evalValueCustom(ui8* dest, YAML::Node& value, const Value* decl, Environment* env) {
        // Test Arguments
        if (decl->typeName.empty()) return Error::TYPE_NOT_FOUND;

        // Attempt To Find The Type
        Type* type = env->getType(decl->typeName);
        if (type == nullptr) return Error::TYPE_NOT_FOUND;

        return parse(dest, value, env, type);
    }
    inline Error evalValueEnum(ui8* dest, YAML::Node& value, const Value* decl, Environment* env) {
        // Test Arguments
        if (decl->typeName.empty()) return Error::TYPE_NOT_FOUND;

        // Attempt To Find The Type
        Enum* type = env->getEnum(decl->typeName);
        if (type == nullptr) return Error::TYPE_NOT_FOUND;
        type->setValue(dest, value.as<nString>());
        return Error::NONE;
    }
    inline Error evalValuePtr(void** dest, YAML::Node& value, const Value* decl, Environment* env) {
        // The Type We Will Be Allocating
        nString typeName = decl->typeName;
        if (typeName.empty()) {
            auto kvp = basicTypes.find(decl->interiorValue->type);
            if (kvp != basicTypes.end()) typeName = kvp->second;
            else typeName = decl->interiorValue->typeName;
        }

        if (value["__TYPE__"]) {
            typeName = value["__TYPE__"].as<nString>();
        }

        // Test Arguments
        if (typeName.empty()) return Error::TYPE_NOT_FOUND;

        // Attempt To Find The Type
        Type* type = env->getType(typeName);
        if (type == nullptr) return Error::TYPE_NOT_FOUND;

        // Undefined Behavior Time :)
        *dest = new ui8[type->getSizeInBytes()]();

        return parse((ui8*)*dest, value, env, type);
    }
    inline Error evalValueArray(ArrayBase* dest, YAML::Node& value, const Value* decl, Environment* env) {
        nString typeName = decl->typeName;
        if (typeName.empty()) {
            auto kvp = basicTypes.find(decl->interiorValue->type);
            if (kvp != basicTypes.end()) typeName = kvp->second;
            else typeName = decl->interiorValue->typeName;
        }

        YAML::Node nArray;
        if (value.IsSequence()) {
            nArray = value;
        } else {
            if (value["__DATA__"]) {
                nArray = value["__DATA__"];
                if (!nArray.IsSequence()) return Error::BAD_VALUE;
                if (value["__TYPE__"]) {
                    typeName = value["__TYPE__"].as<nString>();
                }
            }
        }

        // Test Arguments
        if (typeName.empty()) return Error::TYPE_NOT_FOUND;

        // Attempt To Find The Type
        Type* type = env->getType(typeName);
        if (type == nullptr) {
            return Error::TYPE_NOT_FOUND;
        }
        *dest = ArrayBase(type->getSizeInBytes());
        i32 len = nArray.size();
        if (nArray.size() > 0) {
            dest->setData(nArray.size());
            ui8* newDest = &dest->at<ui8>(0);
            for (i32 i = 0; i < dest->getLength(); i++) {
                YAML::Node tmp = nArray[i];
                evalData(newDest, decl->interiorValue, tmp, env);
                newDest += type->getSizeInBytes();
            }
        }
        return Error::TYPE_NOT_FOUND;
    }

    void evalData(ui8* dest, const Value* decl, YAML::Node &node, Environment* env) {
#define KEG_EVAL_CASE_NUM(ENUM, TYPE) \
        case BasicType::ENUM: *((TYPE*)dest) = node.as<TYPE>(); break; \
        case BasicType::ENUM##_V2: *((TYPE##v2*)dest) = node.as<TYPE##v2>(); break; \
        case BasicType::ENUM##_V3: *((TYPE##v3*)dest) = node.as<TYPE##v3>(); break; \
        case BasicType::ENUM##_V4: *((TYPE##v4*)dest) = node.as<TYPE##v4>(); break
        switch (decl->type) {
            KEG_EVAL_CASE_NUM(I8, i8);
            KEG_EVAL_CASE_NUM(I16, i16);
            KEG_EVAL_CASE_NUM(I32, i32);
            KEG_EVAL_CASE_NUM(I64, i64);
            KEG_EVAL_CASE_NUM(UI8, ui8);
            KEG_EVAL_CASE_NUM(UI16, ui16);
            KEG_EVAL_CASE_NUM(UI32, ui32);
            KEG_EVAL_CASE_NUM(UI64, ui64);
            KEG_EVAL_CASE_NUM(F32, f32);
            KEG_EVAL_CASE_NUM(F64, f64);
        case BasicType::BOOL:
            *((bool*)dest) = node.as<bool>();
            break;
        case BasicType::ARRAY:
            evalValueArray((ArrayBase*)dest, node, decl, env);
            break;
        case BasicType::C_STRING:
            *((cString*)dest) = node.as<cString>();
            break;
        case BasicType::STRING:
            *((nString*)dest) = node.as<nString>();
            break;
        case BasicType::PTR:
            evalValuePtr((void**)dest, node, decl, env);
            break;
        case BasicType::ENUM:
            evalValueEnum((ui8*)dest, node, decl, env);
            break;
        case BasicType::CUSTOM:
            evalValueCustom((ui8*)dest, node, decl, env);
            break;
        default:
            break;
        }
    }

    Error parse(ui8* dest, YAML::Node& data, Environment* env, Type* type) {
        if (data.Type() != YAML::NodeType::Map) return Error::BAD_VALUE;

        // Attempt To Redefine Type
        if (data["__TYPE__"]) {
            YAML::Node nodeType = data["__TYPE__"];
            if (nodeType.IsScalar()) {
                Type* nType = env->getType(nodeType.as<nString>());
                if (nType) {
                    type = nullptr;
                }
            }
        }

        // We Need A Type
        if (!type) return Error::TYPE_NOT_FOUND;

        // Iterate Values
        for (auto nv : data) {
            nString valName = nv.first.as<nString>();
            const Value* v = type->getValue(valName);
            if (v) evalData(dest + v->offset, v, nv.second, env);
        }
        return Error::NONE;
    }
    bool write(const ui8* src, YAML::Emitter& e, Environment* env, Type* type) {
        // TODO: Add Ptr And Array Support

        Type* interiorType = nullptr;
        Enum* interiorEnum = nullptr;

        auto iter = type->getIter();
        while (iter != type->getIterEnd()) {
            // Write The Key
            e << YAML::Key << iter->first;
            e << YAML::Value;

            // Write The Value
            Value v = iter->second;
            const ui8* data = src + v.offset;
            switch (v.type) {
            case BasicType::ENUM:
                // Attempt To Find The Enum
                interiorEnum = env->getEnum(v.typeName);
                if (interiorEnum == nullptr) {
                    return false;
                }
                // Write Enum String
                e << interiorEnum->getValue(data);
                break;
            case BasicType::CUSTOM:
                // Attempt To Find The Type
                interiorType = env->getType(v.typeName);
                if (interiorType == nullptr) {
                    return false;
                }
                // Write To Interior Node
                e << YAML::BeginMap;
                write(data, e, env, interiorType);
                e << YAML::EndMap;
                break;
            case BasicType::PTR:
                break;
            case BasicType::ARRAY:
                // Attempt To Find The Type
                interiorType = env->getType(v.interiorValue->typeName);
                if (interiorType == nullptr) return false;

                // Write To Interior Array
                //writeArray(*(ArrayBase*)data, e, env, interiorType);
                break;
            case BasicType::BOOL:
                e << *(bool*)data;
                break;
            case BasicType::C_STRING:
                e << *(cString*)data;
                break;
            case BasicType::STRING:
                e << (*(nString*)data).c_str();
                break;

                // For when we want to cast TYPE to C_TYPE
#define EMIT_CAST(TYPE, C_TYPE) \
            case BasicType::TYPE: e << (C_TYPE)*data; break; \
            case BasicType::TYPE##_V2: e << C_TYPE##v2(*data); break; \
            case BasicType::TYPE##_V3: e << C_TYPE##v3(*data); break; \
            case BasicType::TYPE##_V4: e << C_TYPE##v4(*data); break
                // For when we want to interpret TYPE as C_TYPE
#define EMIT_NUM(TYPE, C_TYPE) \
            case BasicType::TYPE: e << *(C_TYPE*)data; break; \
            case BasicType::TYPE##_V2: e << *(C_TYPE##v2*)data; break; \
            case BasicType::TYPE##_V3: e << *(C_TYPE##v3*)data; break; \
            case BasicType::TYPE##_V4: e << *(C_TYPE##v4*)data; break
                EMIT_CAST(I8, i32); // Prints out bytes as ints
                EMIT_NUM(I16, i16);
                EMIT_NUM(I32, i32);
                EMIT_NUM(I64, i64);
                EMIT_CAST(UI8, ui32); // Prints out bytes as ints
                EMIT_NUM(UI16, ui16);
                EMIT_NUM(UI32, ui32);
                EMIT_NUM(UI64, ui64);
                EMIT_NUM(F32, f32);
                EMIT_NUM(F64, f64);
            default:
                break;
            }
            iter++;
        }
        return true;
    }
    bool writeArray(ArrayBase src, YAML::Emitter& e, Environment* env, Type* type) {
        e << YAML::BeginSeq;

        e << YAML::EndSeq;
        return true;
    }

    // Our Global Environment :)
    Environment* kegGE = nullptr;
    Environment* getGlobalEnvironment() {
        if (!kegGE) kegGE = new Environment;
        return kegGE;
    }
}
