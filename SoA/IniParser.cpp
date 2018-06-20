#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <memory>

#include "IniParser.h"

// These Are All The Types That Can Be Parsed
enum PTYPE {
    UNKNOWN,
    CHAR,
    SHORT,
    INT,
    LONG,
    FLOAT,
    DOUBLE,
    STRING
};

// These Are Used In The File To Denote Type
#define CHAR_BYTE 'b'
#define CHAR_SHORT 'h'
#define CHAR_INT 'i'
#define CHAR_LONG 'l'
#define CHAR_FLOAT 'f'
#define CHAR_DOUBLE 'd'
#define CHAR_STRING 's'

long GetFileSize(const char* filename) {
    // Check The File Path
    if(!filename) return -1;

    // Use File Information For The Size
    struct stat stat_buf;
    int rc = stat(filename, &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

// Helpers For Navigating Within The String
inline char* GoToNonWhitespaceML(char* s) {
    // Ensure Argument
    if(!s) return 0;

    while(true) {
        switch(*s) {
            // All Whitespace Is Skipped
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                s++;
            default:
                return s;
        }
    }
}
inline char* GoToNonWhitespace(char* s) {
    // Ensure Argument
    if(!s) return 0;

    while(true) {
        switch(*s) {
            // Only Spaces And Tabs Count As Whitespace
            case ' ':
            case '\t':
                s++;
            default:
                return s;
        }
    }
}

// Custom Numerical Converters For Integers
unsigned long int ExtractHex(char* s) {
    // Ensure Argument
    if(!s) return 0;

    // Get Rid Of Beginning Whitespace
    s = GoToNonWhitespace(s);

    // Check For Inverting Values
    bool flip = false;
    if(*s == '~') {
        flip = true;
        s++;
    }

    // Parse The Value
    int i = 0;
    unsigned long int num = 0;
    while(true) {
        switch(s[i]) {
            case 0:
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                // The End Of The String Has Been Reached
                return flip ? ~num : num;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                // Append Values In Hex Form
                num <<= 4;
                num |= s[i] - '0';
                break;
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
                // Append Values In Hex Form
                num <<= 4;
                num |= s[i] - 'a' + 10;
                break;
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
                // Append Values In Hex Form
                num <<= 4;
                num |= s[i] - 'A' + 10;
                break;
            default:
                // Other Characters Are Unsupported
#ifdef DEBUG
                printf("Found Unsupported Character While Parsing Hex Value\n");
#endif // DEBUG
                return flip ? ~num : num;
        }

        // Move To The Next Character
        i++;
    }
}
unsigned long int ExtractOctal(char* s) {
    // Ensure Argument
    if(!s) return 0;

    // Get Rid Of Beginning Whitespace
    s = GoToNonWhitespace(s);

    // Check For Hex Value
    if(s[0] == 'x') return ExtractHex(s + 1);

    // Check For Inverting Values
    bool flip = false;
    if(*s == '~') {
        flip = true;
        s++;
    }

    // Parse The Value
    int i = 0;
    unsigned long int num = 0;
    while(true) {
        switch(s[i]) {
            case 0:
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                // The End Of The String Has Been Reached
                return flip ? ~num : num;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                // Append Values In Octal Form
                num <<= 3;
                num |= s[i] - '0';
                break;
            default:
                // Other Characters Are Unsupported
#ifdef DEBUG
                printf("Found Unsupported Character While Parsing Hex Value\n");
#endif // DEBUG
                return flip ? ~num : num;
        }

        // Move To The Next Character
        i++;
    }
}
unsigned long int ExtractNumber(char* s) {
    // Ensure Argument
    if(!s) return 0;

    // Get Rid Of Beginning Whitespace
    s = GoToNonWhitespace(s);

    // Check For Octal Value
    if(s[0] == '0') return ExtractOctal(s + 1);

    // Check For Inverting Values
    bool flip = false;
    if(*s == '-') {
        flip = true;
        s++;
    }

    // Parse The Value
    int i = 0;
    unsigned long int num = 0;
    while(true) {
        switch(s[i]) {
            case 0:
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                // The End Of The String Has Been Reached
                return flip ? -num : num;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                // Append Values In Octal Form
                num *= 10;
                num += s[i] - '0';
                break;
            default:
                // Other Characters Are Unsupported
#ifdef DEBUG
                printf("Found Unsupported Character While Parsing Hex Value\n");
#endif // DEBUG
                return flip ? ~num : num;
        }

        // Move To The Next Character
        i++;
    }
}

// Helpers To Extract Information From An INI Value
PTYPE ExtractType(char*& s) {
    // Ensure Argument
    if(!s) return UNKNOWN;

    // Go To The First Character In The Type
    s = GoToNonWhitespaceML(s);
    if(*s == 0) return UNKNOWN;

    // Transfer Chars To An Enum
    switch(*s) {
        case CHAR_BYTE: return CHAR;
        case CHAR_SHORT: return SHORT;
        case CHAR_INT: return INT;
        case CHAR_LONG: return LONG;
        case CHAR_FLOAT: return FLOAT;
        case CHAR_DOUBLE: return DOUBLE;
        case CHAR_STRING: return STRING;
        default: return UNKNOWN;
    }
}
char* ExtractValue(char*& s) {
    int cap = 10, count = 0;
    char* v = (char*)malloc(cap);
    while(*s != '}' && *s != 0) {
        // Check For Escape
        if(*s == '\\') {
            s++;
            if(*s == 0) break;
        }

        // Append The Character
        v[count++] = *s;
        s++;

        // Check For String Capacity
        if(count == cap) {
            cap <<= 1;
            v = (char*)realloc(v, cap);
        }
    }

    // Check For A Bad Value
    if(*s == 0) {
        free(v);
        return 0;
    }
    else s++;

    // Null-terminated
    v[count] = 0;
    return v;
}

// This Actually Parses The File
int ByteBlit(const char* file, void* dst, int maxSize) {
    // Keep Track Of The Current Number Of Bytes Read
    int curSize = 0;
    char* bytes = (char*)dst;

    // Attempt To Open The File
    FILE* f;
    errno_t err = fopen_s(&f, file, "r");
    if(err) {
#ifdef DEBUG
        printf("Could Not Open INI File\nError: %d", err);
#endif // DEBUG
        throw err;
    }

    long fs = GetFileSize(file);
    char* data = (char*)malloc(fs);
    fread_s(data, fs, 1, fs, f);

    while(curSize < maxSize) {
        // Extract The Type Of The Data
        PTYPE pt = ExtractType(data);
        if(pt == PTYPE::UNKNOWN) break;

        // Move To The First Bracket
        while(*data != '{' && *data != 0) data++;
        if(*data == 0) break;
        data++;

        // Extract The Value Into A New String
        char* value = ExtractValue(data);
        if(!value) break;

#define APPENDER(TYPE, SIZE, FUNC) \
        if(curSize + 1 <= maxSize) { \
            TYPE val = (TYPE)FUNC(value); \
            memcpy_s(bytes + curSize, SIZE, &val, SIZE); \
            curSize += SIZE; \
        } \
        else maxSize = -1; \
        break

        // Copy Value Data Into The Destination Making Sure To Not Go Over Alotted Size
        switch(pt) {
            case PTYPE::CHAR: APPENDER(unsigned char, 1, ExtractNumber);
            case PTYPE::SHORT: APPENDER(unsigned short, 2, ExtractNumber);
            case PTYPE::INT: APPENDER(unsigned int, 4, ExtractNumber);
            case PTYPE::LONG: APPENDER(unsigned long int, 8, ExtractNumber);
            case PTYPE::FLOAT: APPENDER(float, 4, atof);
            case PTYPE::DOUBLE: APPENDER(double, 8, atof);
            case PTYPE::STRING: APPENDER(char*, sizeof(char*), );
            default: break;
        }
    }

    // Return Amount Of Data That Was Read
    return curSize;
}

// A Test Case
#ifdef TEST_INIPARSER
struct MYSTR {
public:
    int V1;
    int V2;
    int V3;
    int V4;
    char* VStr;
};
int test_main(int argc, char** argv) {
    MYSTR v;
    int bytes = ByteBlit("src/test/data.ini", &v, sizeof(MYSTR));
    printf(v.VStr);
    return 0;
}
#endif // TEST_INIPARSER
