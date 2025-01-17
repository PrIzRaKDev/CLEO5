// some utilities usefull when creating CLEO plugins
// requires adding "CPools.cpp" from GTA Plugin SDK to the project

#pragma once
#include "CLEO.h"
#include "CFileMgr.h" // from GTA Plugin SDK
#include "CPools.h" // from GTA Plugin SDK
#include "shellapi.h" // game window minimize/maximize support
#include <string>
#include <wtypes.h>

namespace CLEO
{
    /*
    TRACE(format,...) // log to file. Can be displayed on screen with change in DebugUtils.ini
    LOG_WARNING(script, format, ...) // warning text on screen and in log file. Not displayed for scripts in 'legacy' mode
    SHOW_ERROR(a,...) // message box, log to file

    Macros to use inside opcode handler functions. Performs types validation, printing warnings and suspending script on critical errors.
    Please mind those might expand into multiple lines, so should, for example, not be used as body of 'if' statements without brackets!
    
    OPCODE_CONDITION_RESULT(value) // set result
    OPCODE_SKIP_PARAMS(count) // ignore X params

    OPCODE_PEEK_PARAM_TYPE() // get param type without advancing the script
    
    // reading opcode input arguments
    OPCODE_READ_PARAM_BOOL()
    OPCODE_READ_PARAM_INT8()
    OPCODE_READ_PARAM_UINT8()
    OPCODE_READ_PARAM_INT16()
    OPCODE_READ_PARAM_UINT16()
    OPCODE_READ_PARAM_INT()
    OPCODE_READ_PARAM_UINT()
    OPCODE_READ_PARAM_FLOAT()
    OPCODE_READ_PARAM_ANY32() // get raw data of any simple-type value (practically integers and floats)
    OPCODE_READ_PARAM_STRING(varName) // reads param and creates const char* variable named 'varName' with pointer to null-terminated string
    OPCODE_READ_PARAM_STRING_LEN(varName, maxLength) // same as above, but text length is clamped to maxLength
    OPCODE_READ_PARAM_STRING_FORMATTED(varName) // reads "format" string argument, then all var-args. Creates variable named 'varName' containing formatted text. Creates 'varNameOk' variable with pointer to the text, or nullptr if user provided invalid arguments
    OPCODE_READ_PARAMS_FORMATTED(format, varName) // reads all var-args and tries to put them into formatted string. Creates variable named 'varName' containing formatted text. Creates 'varNameOk' variable with pointer to the text, or nullptr if user provided invalid arguments
    OPCODE_READ_PARAM_FILEPATH(varName) // reads param and creates const char* variable named 'varName' with pointer to resolved, null-terminated, filepath
    OPCODE_READ_PARAM_PTR() // read and validate memory address argument
    OPCODE_READ_PARAM_OBJECT_HANDLE() // read and validate game object handle
    OPCODE_READ_PARAM_PED_HANDLE() // read and validate character (ped/actor) handle
    OPCODE_READ_PARAM_VEHICLE_HANDLE() // read and validate vehicle handle

    // for opcodes with mixed params order, where 'strore_to' occurs before input arguments 
    OPCODE_READ_PARAM_OUTPUT_VAR_INT() // get pointer to integer variable param to write result later
    OPCODE_READ_PARAM_OUTPUT_VAR_FLOAT() // get pointer to float variable param to write result later
    OPCODE_READ_PARAM_OUTPUT_VAR_ANY32() // get pointer to simple-type variable param to write result later
    OPCODE_READ_PARAM_OUTPUT_VAR_STRING() // returns instance of StringParamBufferInfo used to write string param later

    // writing opcode output/result data
    OPCODE_WRITE_PARAM_BOOL(value)
    OPCODE_WRITE_PARAM_INT8(value)
    OPCODE_WRITE_PARAM_UINT8(value)
    OPCODE_WRITE_PARAM_INT16(value)
    OPCODE_WRITE_PARAM_UINT16(value)
    OPCODE_WRITE_PARAM_INT(value)
    OPCODE_WRITE_PARAM_UINT(value)
    OPCODE_WRITE_PARAM_FLOAT(value)
    OPCODE_WRITE_PARAM_ANY32(value) // write raw data into simple-type variable (practically integers and floats)
    OPCODE_WRITE_PARAM_STRING(value)
    OPCODE_WRITE_PARAM_STRING_INFO(info, value) // write param using info object revceived from OPCODE_READ_PARAM_OUTPUT_VAR_STRING
    OPCODE_WRITE_PARAM_PTR(value) // memory address
    */

    static bool IsLegacyScript(CLEO::CRunningScript* thread)
    {
        return CLEO_GetScriptVersion(thread) < CLEO_VER_5;
    }
    
    // this plugin's config file
    static std::string GetConfigFilename()
    {
        std::string configFile = CFileMgr::ms_rootDirName;
        if (!configFile.empty() && configFile.back() != '\\') configFile.push_back('\\');

        configFile += "cleo\\cleo_plugins\\" TARGET_NAME ".ini";

        return configFile;
    }

    static std::string StringPrintf(const char* format, ...)
    {
        va_list args;

        va_start(args, format);
        auto len = std::vsnprintf(nullptr, 0, format, args) + 1;
        va_end(args);

        std::string result(len, '\0');

        va_start(args, format);
        std::vsnprintf(result.data(), result.length(), format, args);
        va_end(args);

        return result;
    }

    static std::string ScriptInfoStr(CLEO::CRunningScript* thread)
    {
        std::string info(1024, '\0');
        CLEO_GetScriptInfoStr(thread, true, info.data(), info.length());
        return std::move(info);
    }

    static bool IsObjectHandleValid(DWORD handle)
    {
        // get handle info
        auto flags = handle & 0xFF;
        auto index = handle >> 8;

        if (index >= (DWORD)CPools::ms_pObjectPool->m_nSize)
            return false; // index out of bounds

        if (CPools::ms_pObjectPool->m_byteMap[index].IntValue() != flags)
            return false; // flags mismatch

        return true;
    }

    static bool IsPedHandleValid(DWORD handle)
    {
        // get handle info
        auto flags = handle & 0xFF;
        auto index = handle >> 8;

        if (index >= (DWORD)CPools::ms_pPedPool->m_nSize)
            return false; // index out of bounds

        if (CPools::ms_pPedPool->m_byteMap[index].IntValue() != flags)
            return false; // flags mismatch

        return true;
    }

    static bool IsVehicleHandleValid(DWORD handle)
    {
        // get handle info
        auto flags = handle & 0xFF;
        auto index = handle >> 8;

        if (index >= (DWORD)CPools::ms_pVehiclePool->m_nSize)
            return false; // index out of bounds

        if (CPools::ms_pVehiclePool->m_byteMap[index].IntValue() != flags)
            return false; // flags mismatch

        return true;
    }

    static const char* TraceVArg(CLEO::eLogLevel level, const char* format, va_list args)
    {
        static char szBuf[1024];
        vsprintf(szBuf, format, args); // put params into format
        CLEO_Log(level, szBuf);
        return szBuf;
    }

    static void Trace(CLEO::eLogLevel level, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        TraceVArg(level, format, args);
        va_end(args);
    }

    static void Trace(const CLEO::CRunningScript* thread, CLEO::eLogLevel level, const char* format, ...)
    {
        if (thread != nullptr && CLEO_GetScriptVersion(thread) < CLEO::eCLEO_Version::CLEO_VER_5)
        {
            return; // do not log this in older versions
        }

        va_list args;
        va_start(args, format);
        TraceVArg(level, format, args);
        va_end(args);
    }

    static void ShowError(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        auto msg = TraceVArg(CLEO::eLogLevel::Error, format, args);
        va_end(args);

        QUERY_USER_NOTIFICATION_STATE pquns;
        SHQueryUserNotificationState(&pquns);
        bool fullscreen = (pquns == QUNS_BUSY) || (pquns == QUNS_RUNNING_D3D_FULL_SCREEN) || (pquns == QUNS_PRESENTATION_MODE);

        if (fullscreen)
        {
            PostMessage(NULL, WM_SYSCOMMAND, SC_MINIMIZE, 0);
            ShowWindow(NULL, SW_MINIMIZE);
        }

        MessageBox(NULL, msg, "CLEO error", MB_SYSTEMMODAL | MB_TOPMOST | MB_ICONERROR | MB_OK);

        if (fullscreen)
        {
            PostMessage(NULL, WM_SYSCOMMAND, SC_RESTORE, 0);
            ShowWindow(NULL, SW_RESTORE);
        }
    }

    static std::string GetParamInfo(int offset = 0)
    {
        std::string info;
        info.resize(32);
        CLEO_GetScriptParamInfoStr(offset, info.data(), info.length());
        return info;
    }

    struct StringParamBufferInfo
    {
        char* data = nullptr;
        int size = 0;
        BOOL needTerminator = false;
    };

    static void MemPatchJump(size_t position, void* jumpTarget)
    {
        DWORD oldProtect;
        VirtualProtect((LPVOID)position, 5, PAGE_EXECUTE_READWRITE, &oldProtect);

        *(BYTE*)position = 0xE9; // asm: jmp
        position += sizeof(BYTE);

        *(DWORD*)position = (DWORD)jumpTarget - position - 4;
    }

    static void* MemPatchCall(size_t position, void* newFunction)
    {
        DWORD oldProtect;
        VirtualProtect((LPVOID)position, 5, PAGE_EXECUTE_READWRITE, &oldProtect);

        *(BYTE*)position = 0xE8; // asm: call
        position += sizeof(BYTE);

        DWORD original = *(DWORD*)position + position + 4;
        *(DWORD*)position = (DWORD)newFunction - position - 4;

        return (void*)original;
    }

    #define TRACE(format,...) {CLEO::Trace(CLEO::eLogLevel::Default, format, __VA_ARGS__);}
    #define LOG_WARNING(script, format, ...) {CLEO::Trace(script, CLEO::eLogLevel::Error, format, __VA_ARGS__);}
    #define SHOW_ERROR(a,...) {CLEO::ShowError(a, __VA_ARGS__);}

    const size_t MinValidAddress = 0x10000; // used for validation of pointers received from scripts. First 64kb are for sure reserved by Windows.
    #define OPCODE_VALIDATE_POINTER(x) if((size_t)x <= MinValidAddress) { SHOW_ERROR("Invalid '0x%X' pointer argument in script %s \nScript suspended.", x, ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_CONDITION_RESULT(value) CLEO_SetThreadCondResult(thread, value);

    // opcode param handling utils internal
    static SCRIPT_VAR* _paramsArray = nullptr;
    static eDataType _lastParamType = eDataType::DT_END;
    static eArrayDataType _lastParamArrayType = eArrayDataType::ADT_NONE;

    static SCRIPT_VAR& _readParam(CRunningScript* thread)
    {
        _lastParamType = thread->PeekDataType();
        _lastParamArrayType = IsArray(_lastParamType) ? thread->PeekArrayDataType() : eArrayDataType::ADT_NONE;

        CLEO_RetrieveOpcodeParams(thread, 1);
        if (_paramsArray == nullptr) _paramsArray = CLEO_GetOpcodeParamsArray();
        return _paramsArray[0];
    }

    static SCRIPT_VAR& _readParamFloat(CRunningScript* thread)
    {
        auto& var = _readParam(thread);

        // people tend to use '0' instead '0.0' when providing literal float params in scripts
        // binary these are equal, so can be allowed
        if (var.dwParam == 0)
        {
            // pretend it was float type
            if (IsImmInteger(_lastParamType)) _lastParamType = eDataType::DT_FLOAT;
            if (_lastParamArrayType == eArrayDataType::ADT_INT) _lastParamArrayType = eArrayDataType::ADT_FLOAT;
        }

        return var;
    }

    static SCRIPT_VAR* _readParamVariable(CRunningScript* thread)
    {
        _lastParamType = thread->PeekDataType();
        _lastParamArrayType = IsArray(_lastParamType) ? thread->PeekArrayDataType() : eArrayDataType::ADT_NONE;

        return CLEO_GetPointerToScriptVariable(thread);
    }

    static StringParamBufferInfo _readParamStringInfo(CRunningScript* thread)
    {
        _lastParamType = thread->PeekDataType();
        _lastParamArrayType = IsArray(_lastParamType) ? thread->PeekArrayDataType() : eArrayDataType::ADT_NONE;

        StringParamBufferInfo result;
        CLEO_ReadStringParamWriteBuffer(thread, &result.data, &result.size, &result.needTerminator);
        return result;
    }

    static void _writeParamPtr(CRunningScript* thread, void* valuePtr)
    {
        _lastParamType = thread->PeekDataType();
        _lastParamArrayType = IsArray(_lastParamType) ? thread->PeekArrayDataType() : eArrayDataType::ADT_NONE;

        if (_paramsArray == nullptr) _paramsArray = CLEO_GetOpcodeParamsArray();
        _paramsArray[0].pParam = valuePtr;
        CLEO_RecordOpcodeParams(thread, 1);
    }

    template<typename T> static void _writeParam(CRunningScript* thread, T value)
    {
        _lastParamType = thread->PeekDataType();
        _lastParamArrayType = IsArray(_lastParamType) ? thread->PeekArrayDataType() : eArrayDataType::ADT_NONE;

        if (_paramsArray == nullptr) _paramsArray = CLEO_GetOpcodeParamsArray();
        _paramsArray[0].dwParam = 0;
        memcpy(&_paramsArray[0], &value, sizeof(T));
        CLEO_RecordOpcodeParams(thread, 1);
    }

    static inline bool _paramWasInt(bool output = false)
    {
        if (_lastParamArrayType != eArrayDataType::ADT_NONE) return _lastParamArrayType == eArrayDataType::ADT_INT;
        if (IsVariable(_lastParamType)) return true;
        if (!output && IsImmInteger(_lastParamType)) return true;
        return false;
    }

    static inline bool _paramWasFloat(bool output = false)
    {
        if (_lastParamArrayType != eArrayDataType::ADT_NONE) return _lastParamArrayType == eArrayDataType::ADT_FLOAT;
        if (IsVariable(_lastParamType)) return true;
        if (!output && IsImmFloat(_lastParamType)) return true;
        return false;
    }

    static inline bool _paramWasString(bool output = false)
    {
        if (_lastParamArrayType != eArrayDataType::ADT_NONE)
        {
            return _lastParamArrayType == eArrayDataType::ADT_STRING ||
                _lastParamArrayType == eArrayDataType::ADT_TEXTLABEL ||
                _lastParamArrayType == eArrayDataType::ADT_INT; // pointer to output buffer
        }

        if (IsVarString(_lastParamType)) return true;
        if (/*!output &&*/ IsImmString(_lastParamType)) return true; // allow writing strings into const addresses

        // pointer to output buffer
        if (IsVariable(_lastParamType)) return true; 

        return false;
    }

    static inline bool _paramWasVariable()
    {
        return IsVariable(_lastParamType);
    }

    static const char* _readParamText(CRunningScript* thread, char* buffer, size_t bufferSize)
    {
        _lastParamType = thread->PeekDataType();
        _lastParamArrayType = IsArray(_lastParamType) ? thread->PeekArrayDataType() : eArrayDataType::ADT_NONE;

        if (!_paramWasString())
        {
            SHOW_ERROR("Input argument %s expected to be string, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), ToKindStr(_lastParamType, _lastParamArrayType), ScriptInfoStr(thread).c_str());
            thread->Suspend();
            _lastParamType = DT_INVALID; // mark error
            return nullptr;
        }

        auto str = CLEO_ReadStringPointerOpcodeParam(thread, buffer, bufferSize); // returns pointer to source data whenever possible
        
        if (str == nullptr) // reading string failed
        {
            auto isVariableInt = IsVariable(_lastParamType) && (_lastParamArrayType == eArrayDataType::ADT_NONE || _lastParamArrayType == eArrayDataType::ADT_INT);
            if ((IsImmInteger(_lastParamType) || isVariableInt) && // pointer argument type?
                CLEO_GetOpcodeParamsArray()->dwParam <= MinValidAddress)
            {
                SHOW_ERROR("Invalid '0x%X' pointer of input string argument %s in script %s", CLEO_GetOpcodeParamsArray()->dwParam, GetParamInfo().c_str(), ScriptInfoStr(thread).c_str());
            }
            else
            {
                // other error
                SHOW_ERROR("Invalid input argument %s in script %s\nScript suspended.", GetParamInfo().c_str(), ScriptInfoStr(thread).c_str());
            }

            thread->Suspend();
            _lastParamType = DT_INVALID; // mark error
            return nullptr;
        }

        return str;
    }

    static bool _writeParamText(CLEO::CRunningScript* thread, const StringParamBufferInfo& target, const char* str)
    {
        if (str != nullptr && (size_t)str <= MinValidAddress)
        {
            SHOW_ERROR("Invalid '0x%X' source pointer of output string argument %s in script %s \nScript suspended.", str, GetParamInfo(1).c_str(), ScriptInfoStr(thread).c_str());
            thread->Suspend();
            return false;
        }

        if ((size_t)target.data <= MinValidAddress)
        {
            SHOW_ERROR("Invalid '0x%X' target pointer of output string argument in script %s \nScript suspended.", target.data, ScriptInfoStr(thread).c_str());
            thread->Suspend();
            return false;
        }

        if (target.size == 0)
        {
            return true; // done
        }

        bool addTerminator = target.needTerminator;
        size_t buffLen = target.size - addTerminator;
        size_t length = str == nullptr ? 0 : strlen(str);

        if (buffLen > length) addTerminator = true; // there is space left for terminator

        length = min(length, buffLen);
        if (length > 0) std::memcpy(target.data, str, length);
        if (addTerminator) target.data[length] = '\0';

        return true;
    }

    static bool _writeParamText(CRunningScript* thread, const char* str)
    {
        _lastParamType = thread->PeekDataType();
        _lastParamArrayType = IsArray(_lastParamType) ? thread->PeekArrayDataType() : eArrayDataType::ADT_NONE;

        if (str != nullptr && (size_t)str <= MinValidAddress)
        {
            SHOW_ERROR("Invalid '0x%X' source pointer of output string argument %s in script %s \nScript suspended.", str, GetParamInfo(1).c_str(), ScriptInfoStr(thread).c_str());
            thread->Suspend();
            return false;
        }

        if (!_paramWasString(true))
        {
            SHOW_ERROR("Output argument %s expected to be variable string, got %s in script %s\nScript suspended.", GetParamInfo(1).c_str(), ToKindStr(_lastParamType, _lastParamArrayType), ScriptInfoStr(thread).c_str());
            thread->Suspend();
            return false;
        }

        if (IsVariable(_lastParamType)) // pointer to buffer
        {
            auto ptr = CLEO_PeekIntOpcodeParam(thread);

            if ((size_t)ptr <= MinValidAddress)
            {
                SHOW_ERROR("Invalid '0x%X' pointer of output string argument %s in script %s \nScript suspended.", ptr, GetParamInfo(1).c_str(), ScriptInfoStr(thread).c_str());
                thread->Suspend();
                return false;
            }
        }
        
        StringParamBufferInfo info;
        CLEO_ReadStringParamWriteBuffer(thread, &info.data, &info.size, &info.needTerminator);
        return _writeParamText(thread, info, str); // done
    }

    #define OPCODE_SKIP_PARAMS(_count) CLEO_SkipOpcodeParams(thread, _count)
    #define OPCODE_PEEK_PARAM_TYPE() thread->PeekDataType()

    // macros for reading opcode input params. Performs type validation, throws error and suspends script if user provided invalid argument type
    // TOD: add range checks for limited size types?

    #define OPCODE_READ_PARAM_BOOL() _readParam(thread).dwParam != false; \
        if (!_paramWasInt()) { SHOW_ERROR("Input argument %s expected to be integer, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_READ_PARAM_INT8() _readParam(thread).cParam; \
        if (!_paramWasInt()) { SHOW_ERROR("Input argument %s expected to be integer, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_READ_PARAM_UINT8() _readParam(thread).ucParam; \
        if (!_paramWasInt()) { SHOW_ERROR("Input argument %s expected to be integer, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_READ_PARAM_INT16() _readParam(thread).wParam; \
        if (!_paramWasInt()) { SHOW_ERROR("Input argument %s expected to be integer, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_READ_PARAM_UINT16() _readParam(thread).usParam; \
        if (!_paramWasInt()) { SHOW_ERROR("Input argument %s expected to be integer, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_READ_PARAM_INT() _readParam(thread).nParam; \
        if (!_paramWasInt()) { SHOW_ERROR("Input argument %s expected to be integer, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_READ_PARAM_UINT() _readParam(thread).dwParam; \
        if (!_paramWasInt()) { SHOW_ERROR("Input argument %s expected to be integer, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_READ_PARAM_FLOAT() _readParamFloat(thread).fParam; \
        if (!IsLegacyScript(thread) && !_paramWasFloat()) { SHOW_ERROR("Input argument %s expected to be float, got %s in script %s\nScript suspended.\n\nTo ignore this error, change the file extension from .cs to .cs4 and restart the game.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_READ_PARAM_ANY32() _readParam(thread).dwParam; \
        if (!_paramWasInt() && !_paramWasFloat()) { SHOW_ERROR("Input argument %s expected to be int or float, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_READ_PARAM_STRING(_varName) char _buff_##_varName[MAX_STR_LEN + 1]; const char* ##_varName = _readParamText(thread, _buff_##_varName, MAX_STR_LEN + 1); if(!_paramWasString()) { return OpcodeResult::OR_INTERRUPT; }

    #define OPCODE_READ_PARAM_STRING_LEN(_varName, _maxLen) char _buff_##_varName[_maxLen + 1]; const char* ##_varName = _readParamText(thread, _buff_##_varName, _maxLen + 1); if(##_varName != nullptr) ##_varName = _buff_##_varName; if(!_paramWasString()) { return OpcodeResult::OR_INTERRUPT; }

    #define OPCODE_READ_PARAM_STRING_FORMATTED(_varName) char _buff_format_##_varName[MAX_STR_LEN + 1]; const char* _format_##_varName = _readParamText(thread, _buff_format_##_varName, MAX_STR_LEN + 1); if(!_paramWasString()) { return OpcodeResult::OR_INTERRUPT; } \
        char _varName[2 * MAX_STR_LEN + 1]; char* _varName##Ok = CLEO_ReadParamsFormatted(thread, _buff_format_##_varName, _varName, sizeof(_varName));

    #define OPCODE_READ_PARAMS_FORMATTED(_format, _varName) char _varName[2 * MAX_STR_LEN + 1]; char* _varName##Ok = CLEO_ReadParamsFormatted(thread, _format, _varName, sizeof(_varName));

    #define OPCODE_READ_PARAM_FILEPATH(_varName) char _buff_##_varName[512]; const char* ##_varName = _readParamText(thread, _buff_##_varName, 512); if(##_varName != nullptr) ##_varName = _buff_##_varName; if(_paramWasString()) CLEO_ResolvePath(thread, _buff_##_varName, 512); else return OpcodeResult::OR_INTERRUPT;

    #define OPCODE_READ_PARAM_PTR() _readParam(thread).pParam; \
        if (!_paramWasInt()) { SHOW_ERROR("Input argument %s expected to be integer, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); } \
        else if (_paramsArray[0].dwParam <= MinValidAddress) { SHOW_ERROR("Invalid pointer '0x%X' input argument %s in script %s \nScript suspended.", _paramsArray[0].dwParam, GetParamInfo().c_str(), ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_READ_PARAM_OBJECT_HANDLE() _readParam(thread).dwParam; \
        if (!_paramWasInt()) { SHOW_ERROR("Input argument %s expected to be integer, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); } \
        else if (!IsObjectHandleValid(_paramsArray[0].dwParam)) { SHOW_ERROR("Invalid object handle '0x%X' input argument %s in script %s \nScript suspended.", _paramsArray[0].dwParam, GetParamInfo().c_str(), ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_READ_PARAM_PED_HANDLE() _readParam(thread).dwParam; \
        if (!_paramWasInt()) { SHOW_ERROR("Input argument %s expected to be integer, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); } \
        else if (!IsPedHandleValid(_paramsArray[0].dwParam)) { SHOW_ERROR("Invalid character handle '0x%X' input argument %s in script %s \nScript suspended.", _paramsArray[0].dwParam, GetParamInfo().c_str(), ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_READ_PARAM_VEHICLE_HANDLE() _readParam(thread).dwParam; \
        if (!_paramWasInt()) { SHOW_ERROR("Input argument %s expected to be integer, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); } \
        else if (!IsVehicleHandleValid(_paramsArray[0].dwParam)) { SHOW_ERROR("Invalid vehicle handle '0x%X' input argument %s in script %s \nScript suspended.", _paramsArray[0].dwParam, GetParamInfo().c_str(), ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_READ_PARAM_OUTPUT_VAR_ANY32() _readParamVariable(thread); \
        if (!_paramWasVariable()) { SHOW_ERROR("Output argument %s expected to be variable int or float, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_READ_PARAM_OUTPUT_VAR_INT() (int*)_readParamVariable(thread); \
        if (!_paramWasVariable()) { SHOW_ERROR("Output argument %s expected to be variable int, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); } \
        if (!_paramWasInt(true)) { SHOW_ERROR("Output argument %s expected to be variable int, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_READ_PARAM_OUTPUT_VAR_FLOAT() (float*)_readParamVariable(thread); \
        if (!_paramWasVariable()) { SHOW_ERROR("Output argument %s expected to be variable float, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); } \
        if (!IsLegacyScript(thread) && !_paramWasFloat(true)) { SHOW_ERROR("Output argument %s expected to be variable float, got %s in script %s\nScript suspended.\n\nTo ignore this error, change the file extension from .cs to .cs4 and restart the game.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_READ_PARAM_OUTPUT_VAR_STRING() _readParamStringInfo(thread); \
        if (!_paramWasString(true)) { SHOW_ERROR("Output argument %s expected to be variable string, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }


    // macros for writing opcode output params. Performs type validation, throws error and suspends script if user provided invalid argument type

    #define OPCODE_WRITE_PARAM_BOOL(_value) _writeParam(thread, _value); \
        if (!_paramWasInt(true)) { SHOW_ERROR("Output argument %s expected to be variable int, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_WRITE_PARAM_INT8(_value) _writeParam(thread, _value); \
        if (!_paramWasInt(true)) { SHOW_ERROR("Output argument %s expected to be variable int, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_WRITE_PARAM_UINT8(_value) _writeParam(thread, _value); \
        if (!_paramWasInt(true)) { SHOW_ERROR("Output argument %s expected to be variable int, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_WRITE_PARAM_INT16(_value) _writeParam(thread, _value); \
        if (!_paramWasInt(true)) { SHOW_ERROR("Output argument %s expected to be variable int, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_WRITE_PARAM_UINT16(_value) _writeParam(thread, _value); \
        if (!_paramWasInt(true)) { SHOW_ERROR("Output argument %s expected to be variable int, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_WRITE_PARAM_INT(_value) _writeParam(thread, _value); \
        if (!_paramWasInt(true)) { SHOW_ERROR("Output argument %s expected to be variable int, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_WRITE_PARAM_UINT(_value) _writeParam(thread, _value); \
        if (!_paramWasInt(true)) { SHOW_ERROR("Output argument %s expected to be variable int, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_WRITE_PARAM_ANY32(_value) _writeParam(thread, _value); \
        if (!_paramWasInt(true) && !_paramWasFloat(true)) { SHOW_ERROR("Output argument %s expected to be int or float variable, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_WRITE_PARAM_FLOAT(_value) _writeParam(thread, _value); \
        if (!IsLegacyScript(thread) && !_paramWasFloat(true)) { SHOW_ERROR("Output argument %s expected to be variable float, got %s in script %s\nScript suspended.\n\nTo ignore this error, change the file extension from .cs to .cs4 and restart the game.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }

    #define OPCODE_WRITE_PARAM_STRING(_value) if(!_writeParamText(thread, _value)) { return OpcodeResult::OR_INTERRUPT; }

    #define OPCODE_WRITE_PARAM_VAR_STRING(_info, _value) if(!_writeParamText(thread, _info, _value)) { return OpcodeResult::OR_INTERRUPT; }

    #define OPCODE_WRITE_PARAM_PTR(_value) _writeParamPtr(thread, (void*)_value); \
        if (!_paramWasInt(true)) { SHOW_ERROR("Output argument %s expected to be variable int, got %s in script %s\nScript suspended.", GetParamInfo().c_str(), CLEO::ToKindStr(_lastParamType, _lastParamArrayType), CLEO::ScriptInfoStr(thread).c_str()); return thread->Suspend(); }
}
