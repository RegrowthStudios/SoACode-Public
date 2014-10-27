#pragma once
#include <yaml-cpp/yaml.h>

namespace Keg {
    // These Are All The Types That Can Be Parsed Directly From YAML
    enum class BasicType {
#define KEG_BASIC_NUM_VEC(TYPE) TYPE, TYPE##_V2, TYPE##_V3, TYPE##_V4
        // Signed Integral Types
        KEG_BASIC_NUM_VEC(I8),
        KEG_BASIC_NUM_VEC(I16),
        KEG_BASIC_NUM_VEC(I32),
        KEG_BASIC_NUM_VEC(I64),
        // Unsigned Integral Types
        KEG_BASIC_NUM_VEC(UI8),
        KEG_BASIC_NUM_VEC(UI16),
        KEG_BASIC_NUM_VEC(UI32),
        KEG_BASIC_NUM_VEC(UI64),
        // Floating-Point Types
        KEG_BASIC_NUM_VEC(F32),
        KEG_BASIC_NUM_VEC(F64),
        // Boolean
        BOOL,
        // Strings
        C_STRING, STRING,
        // Array
        ARRAY, PTR,
        // Enum
        ENUM,
        // Custom Type
        CUSTOM
    };
    extern std::map<BasicType, nString> basicTypes;

    // These Are Errors Generated From Parsing
    enum class Error {
        // Yippee Ki Yay
        NONE,
        // A Custom Type Is Not Defined Yet Or The Document Is Wrong
        TYPE_NOT_FOUND,
        // An Invalid Argument Was Supplied To The Parser
        BAD_ARGUMENT,
        // A Bad Value Was Given In The Data
        BAD_VALUE,
        // Not Enough Information Provided In The Data
        EARLY_EOF
    };

    // A Value Type Bound To An Offset In A Struct Of Data
    struct Value {
    public:
        // Set This As A Basic Parseable Value Type
        static Value basic(BasicType t, i32 off);
        // Set This As A Custom Value Type
        static Value custom(nString t, i32 off, bool isEnum = false);
        // Set This As An Array Of Values
        static Value array(i32 off, const Value& interior);
        static Value array(i32 off, const Keg::BasicType& t);
        // Set This As A Value That Is Accessed Via Pointer
        static Value ptr(i32 off, const Value& interior);
        static Value ptr(i32 off, const Keg::BasicType& t);

        // Type Used To Determine Parsing Method
        BasicType type;

        // If It's A Custom Type
        nString typeName;

        // If It's An Array Or Pointer Type
        Value* interiorValue;

        // Bytes Of Offset Into The Struct To Find The Data
        i32 offset;
    };

    // An Environment Which Holds All The Types That Have Been Defined
    class Environment;

    // A Custom Data Type (Map Container Of Values)
    class Type {
        friend class Environment;
    public:
        // Create A Type With The Specified Size, Name, And Linked To An Optional Extra Environment
        Type(size_t sizeInBytes, const nString& name, Environment* env = nullptr);

        // Add A Value With A Key Attached To It (Values May Have More Than One Key)
        void addValue(const nString& name, const Value& type);

        // Only With Single-Inheritance And For The Brave. Would Not Recommend
        void addSuper(const Type* type, size_t offset = 0);

        // Attempt To Find A Value With
        const Value* getValue(const nString& name) const {
            auto kv = _values.find(name);
            if (kv != _values.end()) return &kv->second;
            else return nullptr;
        }

        // Value Traversal
        std::map<nString, Value>::const_iterator getIter() const {
            return _values.cbegin();
        }
        std::map<nString, Value>::const_iterator getIterEnd() const {
            return _values.cend();
        }

        // Set-Once Accessor
        size_t getSizeInBytes() const {
            return _sizeInBytes;
        }
        Type();

    private:
        size_t _sizeInBytes;
        std::map<nString, Value> _values;
    };

#define KEG_ENUM_MAX_SIZE_IN_BYTES 8
    typedef ui64 EnumType;
    class Enum {
        friend class Environment;
    public:
        Enum(size_t sizeInBytes, const nString& name, Environment* env = nullptr);

        template<typename T>
        void addValue(const nString& s, T v) {
            _values[s] = (EnumType)v;
            _valuesRev[(EnumType)v] = s;
        }
        void setValue(void* data, const nString& s) {
            _fSetter(data, s, this);
        }
        nString getValue(const void* data) {
            return _fGetter(data, this);
        }

        // Value Traversal
        std::map<nString, EnumType>::const_iterator getIter() const {
            return _values.cbegin();
        }
        std::map<nString, EnumType>::const_iterator getIterEnd() const {
            return _values.cend();
        }

        // Set-Once Accessor
        size_t getSizeInBytes() const {
            return _sizeInBytes;
        }
    private:
        static void setValue64(void* data, const nString& s, Enum* e);
        static void setValue32(void* data, const nString& s, Enum* e);
        static void setValue16(void* data, const nString& s, Enum* e);
        static void setValue8(void* data, const nString& s, Enum* e);
        static nString getValue64(const void* data, Enum* e);
        static nString getValue32(const void* data, Enum* e);
        static nString getValue16(const void* data, Enum* e);
        static nString getValue8(const void* data, Enum* e);

        void(*_fSetter)(void*, const nString&, Enum*);
        nString(*_fGetter)(const void*, Enum*);

        size_t _sizeInBytes;
        std::map<nString, EnumType> _values;
        std::map<EnumType, nString> _valuesRev;
    };


#define KEG_BAD_TYPE_ID 0
    const i32 TYPE_NUM_PREDEFINED = 44;

    class Environment {
    public:
        Environment();

        // Add A Type To The Environment
        ui32 addType(const nString& name, Type* type);

        // Get A Type From A Name
        Type* getType(const nString& str) const {
            auto kt = _typesByName.find(str);
            if (kt != _typesByName.end()) return kt->second;
            else return nullptr;
        }
        // Get A Type From An ID
        Type* getType(const ui32& id) const {
            auto kt = _typesByID.find(id);
            if (kt != _typesByID.end()) return kt->second;
            else return nullptr;
        }

        // Add An Enum To The Environment
        ui32 addEnum(const nString& name, Enum* type);

        // Get An Enum From A Name
        Enum* getEnum(const nString& str) const {
            auto kt = _enumsByName.find(str);
            if (kt != _enumsByName.end()) return kt->second;
            else return nullptr;
        }
        // Get An Enum From An ID
        Enum* getEnum(const ui32& id) const {
            auto kt = _enumsByID.find(id);
            if (kt != _enumsByID.end()) return kt->second;
            else return nullptr;
        }
    private:
        // Auto-Incrementing ID Counter
        ui32 _uuid;

        // Basic Parseable Types
        Type _internalTypes[TYPE_NUM_PREDEFINED];

        // Type Dictionaries
        std::map<nString, Type*> _typesByName;
        std::map<ui32, Type*> _typesByID;

        // Enum Dictionaries
        std::map<nString, Enum*> _enumsByName;
        std::map<ui32, Enum*> _enumsByID;
    };

    // Parse String Of Data Into A Destination Given A Type And Optionally A Separate Environment
    Error parse(void* dest, const cString data, Type* type = nullptr, Environment* env = nullptr);
    // Parse String Of Data Into A Destination Given A Type Name And Optionally A Separate Environment
    Error parse(void* dest, const cString data, const nString& typeName, Environment* env = nullptr);
    // Parse String Of Data Into A Destination Given A Type ID And Optionally A Separate Environment
    Error parse(void* dest, const cString data, const ui32& typeID, Environment* env = nullptr);
    Error parse(ui8* dest, YAML::Node& data, Environment* env, Type* type);
    void evalData(ui8* dest, const Value* decl, YAML::Node& node, Environment* env);

    nString write(const void* src, Type* type, Environment* env = nullptr);
    nString write(const void* src, const nString& typeName, Environment* env = nullptr);
    nString write(const void* src, const ui32& typeID, Environment* env = nullptr);
    bool write(const ui8* src, YAML::Emitter& e, Environment* env, Type* type);


    // Get The Global Environment Of Custom Types
    Environment* getGlobalEnvironment();
}

// For Use If The Keg::Type Is Declared In A File
#define KEG_GLOBAL_TYPE_INIT(TYPENAME) init_kts_##TYPENAME
#define KEG_GLOBAL_TYPE(TYPENAME) kts_##TYPENAME
#define KEG_GLOBAL_ENUM_INIT(TYPENAME) init_kes_##TYPENAME
#define KEG_GLOBAL_ENUM(TYPENAME) kes_##TYPENAME

// For Use In Header Files:
#define KEG_TYPE_DECL(TYPENAME) extern Keg::Type KEG_GLOBAL_TYPE(TYPENAME); extern bool KEG_GLOBAL_TYPE_INIT(TYPENAME)
#define KEG_ENUM_DECL(TYPENAME) extern Keg::Enum KEG_GLOBAL_ENUM(TYPENAME); extern bool KEG_GLOBAL_ENUM_INIT(TYPENAME)

// For Use In Source Files:
#define KEG_TYPE_INIT_BEGIN(TYPENAME, STRUCT_TYPE, VAR_NAME) bool kt_init_##TYPENAME##(); \
    Keg::Type KEG_GLOBAL_TYPE(TYPENAME)(sizeof(STRUCT_TYPE), #TYPENAME, nullptr); \
    bool KEG_GLOBAL_TYPE_INIT(TYPENAME) = kt_init_##TYPENAME##(); \
    bool kt_init_##TYPENAME##() MACRO_PARAN_L \
    Keg::Type* VAR_NAME = &KEG_GLOBAL_TYPE(TYPENAME);
#define KEG_ENUM_INIT_BEGIN(TYPENAME, STRUCT_TYPE, VAR_NAME) bool ke_init_##TYPENAME##(); \
    Keg::Enum KEG_GLOBAL_ENUM(TYPENAME)(sizeof(STRUCT_TYPE), #TYPENAME, nullptr); \
    bool KEG_GLOBAL_ENUM_INIT(TYPENAME) = ke_init_##TYPENAME##(); \
    bool ke_init_##TYPENAME##() MACRO_PARAN_L \
    Keg::Enum* VAR_NAME = &KEG_GLOBAL_ENUM(TYPENAME);

// For Less Verbose Type Initializations
#define KEG_TYPE_INIT_DEF_VAR_NAME __keg_t
#define KEG_TYPE_INIT_BEGIN_DEF_VAR(STRUCT_TYPE) KEG_TYPE_INIT_BEGIN(STRUCT_TYPE, STRUCT_TYPE, KEG_TYPE_INIT_DEF_VAR_NAME)
#define KEG_TYPE_INIT_ADD_MEMBER(TYPENAME, TYPE_ENUM, MEMBER) KEG_TYPE_INIT_DEF_VAR_NAME->addValue(#MEMBER, Keg::Value::basic(Keg::BasicType::TYPE_ENUM, offsetof(TYPENAME, MEMBER)))

#define KEG_TYPE_INIT_END \
    return true; \
    MACRO_PARAN_R
#define KEG_ENUM_INIT_END \
    return true; \
    MACRO_PARAN_R

/************************************************************************/
/* --- Header File ---                                                  */
/* KEG_TYPE_DECL(Vec2);                                                 */
/*                                                                      */
/* --- Source File ---                                                  */
/* KEG_TYPE_INIT_BEGIN(Vec2, Vec2, t)                                 */
/*     ... Code ...                                                     */
/* KEG_TYPE_INIT_END                                                    */
/************************************************************************/