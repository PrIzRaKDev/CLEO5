#include "stdafx.h"
#include "CleoBase.h"
#include "CGameVersionManager.h"
#include "CCustomOpcodeSystem.h"
#include "ScmFunction.h"
#include "CTextManager.h"
#include "CModelInfo.h"

#include <tlhelp32.h>
#include <sstream>
#include <forward_list>

#define OPCODE_VALIDATE_STR_ARG_WRITE(x) if((void*)x == nullptr) { SHOW_ERROR("%s in script %s \nScript suspended.", CCustomOpcodeSystem::lastErrorMsg.c_str(), ((CCustomScript*)thread)->GetInfoStr().c_str()); return thread->Suspend(); }
#define OPCODE_READ_FORMATTED_STRING(thread, buf, bufSize, format) if(ReadFormattedString(thread, buf, bufSize, format) == -1) { SHOW_ERROR("%s in script %s \nScript suspended.", CCustomOpcodeSystem::lastErrorMsg.c_str(), ((CCustomScript*)thread)->GetInfoStr().c_str()); return thread->Suspend(); }

namespace CLEO 
{
	template<typename T> inline CRunningScript& operator>>(CRunningScript& thread, T*& pval);
	template<typename T> inline CRunningScript& operator<<(CRunningScript& thread, T* pval);
	template<typename T> inline CRunningScript& operator<<(CRunningScript& thread, memory_pointer pval);
	template<typename T> inline CRunningScript& operator>>(CRunningScript& thread, memory_pointer& pval);


	OpcodeResult __stdcall opcode_0051(CRunningScript * thread); // GOSUB return

	OpcodeResult __stdcall opcode_0A92(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0A93(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0A94(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0A95(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AA0(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AA1(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AA9(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AB0(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AB1(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AB2(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AB3(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AB4(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AB5(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AB6(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AB7(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AB8(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0ABA(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0ABD(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0ABE(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0ABF(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0ACA(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0ACB(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0ACC(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0ACD(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0ACE(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0ACF(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AD0(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AD1(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AD2(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AD3(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AD4(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0ADB(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0ADC(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0ADD(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0ADE(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0ADF(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AE0(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AE1(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AE2(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AE3(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AED(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AEE(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0AEF(CRunningScript *thread);
	OpcodeResult __stdcall opcode_0DD5(CRunningScript* thread); // get_platform
	// 2000 free slot
	// 2001 free slot
	OpcodeResult __stdcall opcode_2002(CRunningScript* thread); // cleo_return_with
	OpcodeResult __stdcall opcode_2003(CRunningScript* thread); // cleo_return_fail

	typedef void(*FuncScriptDeleteDelegateT) (CRunningScript *script);
	struct ScriptDeleteDelegate {
		std::vector<FuncScriptDeleteDelegateT> funcs;
		template<class FuncScriptDeleteDelegateT> void operator+=(FuncScriptDeleteDelegateT mFunc) { funcs.push_back(mFunc); }
		template<class FuncScriptDeleteDelegateT> void operator-=(FuncScriptDeleteDelegateT mFunc) { funcs.erase(std::remove(funcs.begin(), funcs.end(), mFunc), funcs.end()); }
		void operator()(CRunningScript *script)
		{ 
			for (auto& f : funcs)
			{
				// check if function pointer lays within any of currently loaded modules (.asi or .cleo plugins)
				void* ptr = f;
				MODULEENTRY32 module;
				module.dwSize = sizeof(MODULEENTRY32);
				HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
				Module32First(snapshot, &module);
				if (snapshot != INVALID_HANDLE_VALUE)
				{
					size_t count = 0;
					do
					{
						if(ptr >= module.modBaseAddr && ptr <= (module.modBaseAddr + module.modBaseSize))
						{
							f(script);
							break;
						}
					} while (Module32Next(snapshot, &module));
					CloseHandle(snapshot);
				}
			}
			
		}
	};
	ScriptDeleteDelegate scriptDeleteDelegate;
	void RunScriptDeleteDelegate(CRunningScript *script) { scriptDeleteDelegate(script); }

	void(__thiscall * ProcessScript)(CRunningScript*);

	const char * (__cdecl * GetUserDirectory)();
	void(__cdecl * ChangeToUserDir)();
	void(__cdecl * ChangeToProgramDir)(const char *);

	float(__cdecl * FindGroundZ)(float x, float y);
	CMarker		* RadarBlips;

	CHandling	* Handling;

	CPlayerPed * (__cdecl * GetPlayerPed)(DWORD);
	CBaseModelInfo **Models;

	void(__cdecl * SpawnCar)(DWORD);

	CRunningScript* CCustomOpcodeSystem::lastScript = nullptr;
	WORD CCustomOpcodeSystem::lastOpcode = 0;
	WORD* CCustomOpcodeSystem::lastOpcodePtr = nullptr;
	WORD CCustomOpcodeSystem::lastCustomOpcode = 0;
	std::string CCustomOpcodeSystem::lastErrorMsg = {};
	WORD CCustomOpcodeSystem::prevOpcode = 0;
	BYTE CCustomOpcodeSystem::handledParamCount = 0;

	// opcode handler for custom opcodes
	OpcodeResult __fastcall CCustomOpcodeSystem::customOpcodeHandler(CRunningScript *thread, int dummy, WORD opcode)
	{
		prevOpcode = lastOpcode;

		lastScript = thread;
		lastOpcode = opcode;
		lastOpcodePtr = (WORD*)thread->GetBytePointer() - 1; // rewind to the opcode start
		handledParamCount = 0;

		// execute registered callbacks
		OpcodeResult result = OR_NONE;
		for (void* func : GetInstance().GetCallbacks(eCallbackId::ScriptOpcodeProcess))
		{
			typedef OpcodeResult WINAPI callback(CRunningScript*, DWORD);
			result = ((callback*)func)(thread, opcode);

			if(result != OR_NONE)
				break; // processed
		}

		if(result == OR_NONE) // opcode not proccessed yet
		{
			if(opcode > LastCustomOpcode)
			{
				SHOW_ERROR("Opcode [%04X] out of supported range! \nCalled in script %s\nScript suspended.", opcode, ((CCustomScript*)thread)->GetInfoStr().c_str());
				return thread->Suspend();
			}

			CustomOpcodeHandler handler = customOpcodeProc[opcode];
			if(handler != nullptr)
			{
				lastCustomOpcode = opcode;
				return handler(thread);
			}

			// Not registered as custom opcode. Call game's original handler

			if (opcode > LastOriginalOpcode)
			{
				auto extensionMsg = GetInstance().OpcodeInfoDb.GetExtensionMissingMessage(opcode);
				if (!extensionMsg.empty()) extensionMsg = " " + extensionMsg;

				SHOW_ERROR("Custom opcode [%04X] not registered!%s\nCalled in script %s\nPreviously called opcode: [%04X]\nScript suspended.",
					opcode,
					extensionMsg.c_str(),
					((CCustomScript*)thread)->GetInfoStr().c_str(), 
					prevOpcode);

				return thread->Suspend();
			}

			size_t tableIdx = opcode / 100; // 100 opcodes peer handler table
			result = originalOpcodeHandlers[tableIdx](thread, opcode);

			if(result == OR_ERROR)
			{
				auto extensionMsg = GetInstance().OpcodeInfoDb.GetExtensionMissingMessage(opcode);
				if (!extensionMsg.empty()) extensionMsg = " " + extensionMsg;

				SHOW_ERROR("Opcode [%04X] not found!%s\nCalled in script %s\nPreviously called opcode: [%04X]\nScript suspended.",
					opcode,
					extensionMsg.c_str(),
					((CCustomScript*)thread)->GetInfoStr().c_str(), 
					prevOpcode);

				return thread->Suspend();
			}
		}

		// execute registered callbacks
		OpcodeResult callbackResult = OR_NONE;
		for (void* func : GetInstance().GetCallbacks(eCallbackId::ScriptOpcodeProcessFinished))
		{
			typedef OpcodeResult WINAPI callback(CRunningScript*, DWORD, OpcodeResult);
			auto res = ((callback*)func)(thread, opcode, result);

			callbackResult = max(res, callbackResult); // store result with highest value from all callbacks
		}

		return (callbackResult != OR_NONE) ? callbackResult : result;
	}

	void CCustomOpcodeSystem::FinalizeScriptObjects()
	{
		TRACE("Cleaning up script data...");

		GetInstance().CallCallbacks(eCallbackId::ScriptsFinalize);

		// clean up after opcode_0AB1
		ScmFunction::Clear();
	}

	CCustomOpcodeSystem::CCustomOpcodeSystem()
	{
		TRACE("Initializing CLEO core opcodes...");

		CLEO_RegisterOpcode(0x0051, opcode_0051);
		CLEO_RegisterOpcode(0x0A92, opcode_0A92);
		CLEO_RegisterOpcode(0x0A93, opcode_0A93);
		CLEO_RegisterOpcode(0x0A94, opcode_0A94);
		CLEO_RegisterOpcode(0x0A95, opcode_0A95);
		CLEO_RegisterOpcode(0x0AA0, opcode_0AA0);
		CLEO_RegisterOpcode(0x0AA1, opcode_0AA1);
		CLEO_RegisterOpcode(0x0AA9, opcode_0AA9);
		CLEO_RegisterOpcode(0x0AB0, opcode_0AB0);
		CLEO_RegisterOpcode(0x0AB1, opcode_0AB1);
		CLEO_RegisterOpcode(0x0AB2, opcode_0AB2);
		CLEO_RegisterOpcode(0x0AB3, opcode_0AB3);
		CLEO_RegisterOpcode(0x0AB4, opcode_0AB4);
		CLEO_RegisterOpcode(0x0AB5, opcode_0AB5);
		CLEO_RegisterOpcode(0x0AB6, opcode_0AB6);
		CLEO_RegisterOpcode(0x0AB7, opcode_0AB7);
		CLEO_RegisterOpcode(0x0AB8, opcode_0AB8);
		CLEO_RegisterOpcode(0x0ABA, opcode_0ABA);
		CLEO_RegisterOpcode(0x0ABD, opcode_0ABD);
		CLEO_RegisterOpcode(0x0ABE, opcode_0ABE);
		CLEO_RegisterOpcode(0x0ABF, opcode_0ABF);
		CLEO_RegisterOpcode(0x0ACA, opcode_0ACA);
		CLEO_RegisterOpcode(0x0ACB, opcode_0ACB);
		CLEO_RegisterOpcode(0x0ACC, opcode_0ACC);
		CLEO_RegisterOpcode(0x0ACD, opcode_0ACD);
		CLEO_RegisterOpcode(0x0ACE, opcode_0ACE);
		CLEO_RegisterOpcode(0x0ACF, opcode_0ACF);
		CLEO_RegisterOpcode(0x0AD0, opcode_0AD0);
		CLEO_RegisterOpcode(0x0AD1, opcode_0AD1);
		CLEO_RegisterOpcode(0x0AD2, opcode_0AD2);
		CLEO_RegisterOpcode(0x0AD3, opcode_0AD3);
		CLEO_RegisterOpcode(0x0AD4, opcode_0AD4);
		CLEO_RegisterOpcode(0x0ADB, opcode_0ADB);
		CLEO_RegisterOpcode(0x0ADC, opcode_0ADC);
		CLEO_RegisterOpcode(0x0ADD, opcode_0ADD);
		CLEO_RegisterOpcode(0x0ADE, opcode_0ADE);
		CLEO_RegisterOpcode(0x0ADF, opcode_0ADF);
		CLEO_RegisterOpcode(0x0AE0, opcode_0AE0);
		CLEO_RegisterOpcode(0x0AE1, opcode_0AE1);
		CLEO_RegisterOpcode(0x0AE2, opcode_0AE2);
		CLEO_RegisterOpcode(0x0AE3, opcode_0AE3);
		CLEO_RegisterOpcode(0x0AED, opcode_0AED);
		CLEO_RegisterOpcode(0x0AEE, opcode_0AEE);
		CLEO_RegisterOpcode(0x0AEF, opcode_0AEF);

		CLEO_RegisterOpcode(0x0DD5, opcode_0DD5); // get_platform
		
		// 2000, 2001 free
		CLEO_RegisterOpcode(0x2002, opcode_2002); // cleo_return_with
		CLEO_RegisterOpcode(0x2003, opcode_2003); // cleo_return_fail
	}

	void CCustomOpcodeSystem::Inject(CCodeInjector& inj)
	{
		TRACE("Injecting CustomOpcodeSystem...");
		CGameVersionManager& gvm = GetInstance().VersionManager;

		// replace all handlers in original table
		// store original opcode handlers for later use
		_OpcodeHandler* handlersTable = gvm.TranslateMemoryAddress(MA_OPCODE_HANDLER);
		for(size_t i = 0; i < OriginalOpcodeHandlersCount; i++)
		{
			originalOpcodeHandlers[i] = handlersTable[i];
			handlersTable[i] = (_OpcodeHandler)customOpcodeHandler;
		}

		// initialize and apply new handlers table
		for (size_t i = 0; i < CustomOpcodeHandlersCount; i++)
		{
			customOpcodeHandlers[i] = (_OpcodeHandler)customOpcodeHandler;
		}
		MemWrite(gvm.TranslateMemoryAddress(MA_OPCODE_HANDLER_REF), &customOpcodeHandlers);
		MemWrite(0x00469EF0, &customOpcodeHandlers); // TODO: game version translation

		GetUserDirectory = gvm.TranslateMemoryAddress(MA_GET_USER_DIR_FUNCTION);
		ChangeToUserDir = gvm.TranslateMemoryAddress(MA_CHANGE_TO_USER_DIR_FUNCTION);
		ChangeToProgramDir = gvm.TranslateMemoryAddress(MA_CHANGE_TO_PROGRAM_DIR_FUNCTION);
		FindGroundZ = gvm.TranslateMemoryAddress(MA_FIND_GROUND_Z_FUNCTION);
		GetPlayerPed = gvm.TranslateMemoryAddress(MA_GET_PLAYER_PED_FUNCTION);
		Handling = gvm.TranslateMemoryAddress(MA_HANDLING);
		Models = gvm.TranslateMemoryAddress(MA_MODELS);
		SpawnCar = gvm.TranslateMemoryAddress(MA_SPAWN_CAR_FUNCTION);

		// TODO: consider version-agnostic code
		if (gvm.GetGameVersion() == GV_US10)
		{
			// make it compatible with fastman92's limit adjuster (only required for 1.0 US)
			RadarBlips = injector::ReadMemory<CMarker*>(0x583A05 + 2, true);
		}
		else
		{
			RadarBlips = gvm.TranslateMemoryAddress(MA_RADAR_BLIPS);
		}
	}

	CCustomOpcodeSystem::_OpcodeHandler CCustomOpcodeSystem::originalOpcodeHandlers[OriginalOpcodeHandlersCount];
	CCustomOpcodeSystem::_OpcodeHandler CCustomOpcodeSystem::customOpcodeHandlers[CustomOpcodeHandlersCount];
	CustomOpcodeHandler CCustomOpcodeSystem::customOpcodeProc[LastCustomOpcode + 1];

	bool CCustomOpcodeSystem::RegisterOpcode(WORD opcode, CustomOpcodeHandler callback)
	{
		if (opcode > LastCustomOpcode)
		{
			SHOW_ERROR("Can not register [%04X] opcode! Out of supported range.", opcode);
			return false;
		}

		CustomOpcodeHandler& dst = customOpcodeProc[opcode];
		if (*dst != nullptr)
		{
			LOG_WARNING(0, "Opcode [%04X] already registered! Replacing...", opcode);
		}

		dst = callback;
		TRACE("Opcode [%04X] registered", opcode);
		return true;
	}

	inline CRunningScript& operator>>(CRunningScript& thread, DWORD& uval)
	{
		GetScriptParams(&thread, 1);
		uval = opcodeParams[0].dwParam;
		return thread;
	}

	inline CRunningScript& operator<<(CRunningScript& thread, DWORD uval)
	{
		opcodeParams[0].dwParam = uval;
		SetScriptParams(&thread, 1);
		return thread;
	}

	inline CRunningScript& operator>>(CRunningScript& thread, int& nval)
	{
		GetScriptParams(&thread, 1);
		nval = opcodeParams[0].nParam;
		return thread;
	}

	inline CRunningScript& operator<<(CRunningScript& thread, int nval)
	{
		opcodeParams[0].nParam = nval;
		SetScriptParams(&thread, 1);
		return thread;
	}

	inline CRunningScript& operator>>(CRunningScript& thread, float& fval)
	{
		GetScriptParams(&thread, 1);
		fval = opcodeParams[0].fParam;
		return thread;
	}

	inline CRunningScript& operator<<(CRunningScript& thread, float fval)
	{
		opcodeParams[0].fParam = fval;
		SetScriptParams(&thread, 1);
		return thread;
	}

	template<typename T>
	inline CRunningScript& operator>>(CRunningScript& thread, T *& pval)
	{
		GetScriptParams(&thread, 1);
		pval = reinterpret_cast<T *>(opcodeParams[0].pParam);
		return thread;
	}

	template<typename T>
	inline CRunningScript& operator<<(CRunningScript& thread, T *pval)
	{
		opcodeParams[0].pParam = (void *)(pval);
		SetScriptParams(&thread, 1);
		return thread;
	}

	inline CRunningScript& operator>>(CRunningScript& thread, memory_pointer& pval)
	{
		GetScriptParams(&thread, 1);
		pval = opcodeParams[0].pParam;
		return thread;
	}

	template<typename T>
	inline CRunningScript& operator<<(CRunningScript& thread, memory_pointer pval)
	{
		opcodeParams[0].pParam = pval;
		SetScriptParams(&thread, 1);
		return thread;
	}

	const char* ReadStringParam(CRunningScript *thread, char* buff, int buffSize)
	{
		if (buffSize > 0) buff[buffSize - 1] = '\0'; // buffer always terminated
		return GetScriptStringParam(thread, 0, buff, buffSize - 1); // minus terminator
	}

	// write output\result string parameter
	bool WriteStringParam(CRunningScript* thread, const char* str)
	{
		auto target = GetStringParamWriteBuffer(thread);
		return WriteStringParam(target, str);
	}

	bool WriteStringParam(const StringParamBufferInfo& target, const char* str)
	{
		CCustomOpcodeSystem::lastErrorMsg.clear();

		if (str != nullptr && (size_t)str <= CCustomOpcodeSystem::MinValidAddress)
		{
			CCustomOpcodeSystem::lastErrorMsg = stringPrintf("Writing string from invalid '0x%X' pointer", target.data);
			return false;
		}

		if ((size_t)target.data <= CCustomOpcodeSystem::MinValidAddress)
		{
			CCustomOpcodeSystem::lastErrorMsg = stringPrintf("Writing string into invalid '0x%X' pointer argument", target.data);
			return false;
		}

		if (target.size == 0)
		{
			return false;
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

	StringParamBufferInfo GetStringParamWriteBuffer(CRunningScript* thread)
	{
		StringParamBufferInfo result;
		CCustomOpcodeSystem::lastErrorMsg.clear();

		auto paramType = thread->PeekDataType();
		if (IsImmInteger(paramType) || IsVariable(paramType))
		{
			// address to output buffer
			GetScriptParams(thread, 1);

			if (opcodeParams[0].dwParam <= CCustomOpcodeSystem::MinValidAddress)
			{
				CCustomOpcodeSystem::lastErrorMsg = stringPrintf("Writing string into invalid '0x%X' pointer argument", opcodeParams[0].dwParam);
				return result; // error
			}

			result.data = opcodeParams[0].pcParam;
			result.size = 0x7FFFFFFF; // user allocated memory block can be any size
			result.needTerminator = true;

			return result;
		}
		else
		if (IsVarString(paramType))
		{
			switch(paramType)
			{
				// short string variable
				case DT_VAR_TEXTLABEL:
				case DT_LVAR_TEXTLABEL:
				case DT_VAR_TEXTLABEL_ARRAY:
				case DT_LVAR_TEXTLABEL_ARRAY:
					result.data = (char*)GetScriptParamPointer(thread);
					result.size = 8;
					result.needTerminator = false;
					return result;

				// long string variable
				case DT_VAR_STRING:
				case DT_LVAR_STRING:
				case DT_VAR_STRING_ARRAY:
				case DT_LVAR_STRING_ARRAY:
					result.data = (char*)GetScriptParamPointer(thread);
					result.size = 16;
					result.needTerminator = false;
					return result;
			}
		}

		CCustomOpcodeSystem::lastErrorMsg = stringPrintf("Writing string, got argument %s", ToKindStr(paramType));
		CLEO_SkipOpcodeParams(thread, 1); // skip unhandled param
		return result; // error
	}

	// perform 'sprintf'-operation for parameters, passed through SCM
	int ReadFormattedString(CRunningScript *thread, char *outputStr, DWORD len, const char *format)
	{
		unsigned int written = 0;
		const char *iter = format;
		char* outIter = outputStr;
		char bufa[MAX_STR_LEN + 1], fmtbufa[64], *fmta;

		CCustomOpcodeSystem::lastErrorMsg.clear();

		// invalid input arguments
		if(outputStr == nullptr || len == 0)
		{
			CCustomOpcodeSystem::lastErrorMsg = "Need target buffer to read formatted string";
			SkipUnusedVarArgs(thread);
			return -1; // error
		}

		if(len > 1 && format != nullptr)
		{
			while (*iter)
			{
				while (*iter && *iter != '%')
				{
					if (written++ >= len) goto _ReadFormattedString_OutOfMemory;
					*outIter++ = *iter++;
				}

				if (*iter == '%')
				{
					if (iter[1] == '%')
					{
						if (written++ >= len) goto _ReadFormattedString_OutOfMemory;
						*outIter++ = '%'; /* "%%"->'%' */
						iter += 2;
						continue;
					}

					//get flags and width specifier
					fmta = fmtbufa;
					*fmta++ = *iter++;
					while (*iter == '0' ||
						   *iter == '+' ||
						   *iter == '-' ||
						   *iter == ' ' ||
						   *iter == '*' ||
						   *iter == '#')
					{
						if (*iter == '*')
						{
							char *buffiter = bufa;

							//get width
							if (thread->PeekDataType() == DT_END) goto _ReadFormattedString_ArgMissing;
							GetScriptParams(thread, 1);
							_itoa(opcodeParams[0].dwParam, buffiter, 10);
							while (*buffiter)
								*fmta++ = *buffiter++;
						}
						else
							*fmta++ = *iter;
						iter++;
					}

					//get immidiate width value
					while (isdigit(*iter))
						*fmta++ = *iter++;

					//get precision
					if (*iter == '.')
					{
						*fmta++ = *iter++;
						if (*iter == '*')
						{
							char *buffiter = bufa;
							if (thread->PeekDataType() == DT_END) goto _ReadFormattedString_ArgMissing;
							GetScriptParams(thread, 1);
							_itoa(opcodeParams[0].dwParam, buffiter, 10);
							while (*buffiter)
								*fmta++ = *buffiter++;
						}
						else
							while (isdigit(*iter))
								*fmta++ = *iter++;
					}
					//get size
					if (*iter == 'h' || *iter == 'l')
						*fmta++ = *iter++;

					switch (*iter)
					{
					case 's':
					{
						if (thread->PeekDataType() == DT_END) goto _ReadFormattedString_ArgMissing;

						const char* str = ReadStringParam(thread, bufa, sizeof(bufa));
						if(str == nullptr) // read error
						{
							static const char none[] = "(INVALID_STR)";
							str = none;
						}
						
						while (*str)
						{
							if (written++ >= len) goto _ReadFormattedString_OutOfMemory;
							*outIter++ = *str++;
						}
						iter++;
						break;
					}

					case 'c':
						if (written++ >= len) goto _ReadFormattedString_OutOfMemory;
						if (thread->PeekDataType() == DT_END) goto _ReadFormattedString_ArgMissing;
						GetScriptParams(thread, 1);
						*outIter++ = (char)opcodeParams[0].nParam;
						iter++;
						break;

					default:
					{
						/* For non wc types, use system sprintf and append to wide char output */
						/* FIXME: for unrecognised types, should ignore % when printing */
						char *bufaiter = bufa;
						if (*iter == 'p' || *iter == 'P')
						{
							if (thread->PeekDataType() == DT_END) goto _ReadFormattedString_ArgMissing;
							GetScriptParams(thread, 1);
							sprintf(bufaiter, "%08X", opcodeParams[0].dwParam);
						}
						else
						{
							*fmta++ = *iter;
							*fmta = '\0';
							if (*iter == 'a' || *iter == 'A' ||
								*iter == 'e' || *iter == 'E' ||
								*iter == 'f' || *iter == 'F' ||
								*iter == 'g' || *iter == 'G')
							{
								if (thread->PeekDataType() == DT_END) goto _ReadFormattedString_ArgMissing;
								GetScriptParams(thread, 1);
								sprintf(bufaiter, fmtbufa, opcodeParams[0].fParam);
							}
							else
							{
								if (thread->PeekDataType() == DT_END) goto _ReadFormattedString_ArgMissing;
								GetScriptParams(thread, 1);
								sprintf(bufaiter, fmtbufa, opcodeParams[0].pParam);
							}
						}
						while (*bufaiter)
						{
							if (written++ >= len) goto _ReadFormattedString_OutOfMemory;
							*outIter++ = *bufaiter++;
						}
						iter++;
						break;
					}
					}
				}
			}
		}

		if (written >= len)
		{
		_ReadFormattedString_OutOfMemory: // jump here on error

			CCustomOpcodeSystem::lastErrorMsg = stringPrintf("Target buffer too small (%d) to read whole formatted string", len);
			SkipUnusedVarArgs(thread);
			outputStr[len - 1] = '\0';
			return -1; // error
		}

		// still more var-args available
		if (thread->PeekDataType() != DT_END)
		{
			CCustomOpcodeSystem::lastErrorMsg = "More params than slots in formatted string";
			LOG_WARNING(thread, "%s in script %s", CCustomOpcodeSystem::lastErrorMsg.c_str(), ((CCustomScript*)thread)->GetInfoStr().c_str());
		}
		SkipUnusedVarArgs(thread); // skip terminator too

		outputStr[written] = '\0';
		return (int)written;

	_ReadFormattedString_ArgMissing: // jump here on error
		CCustomOpcodeSystem::lastErrorMsg = "Less params than slots in formatted string";
		thread->IncPtr(); // skip vararg terminator
		outputStr[written] = '\0';
		return -1; // error
	}

	OpcodeResult CCustomOpcodeSystem::CleoReturnGeneric(WORD opcode, CRunningScript* thread, bool returnArgs, DWORD returnArgCount, bool strictArgCount)
	{
		auto cs = reinterpret_cast<CCustomScript*>(thread);

		ScmFunction* scmFunc = ScmFunction::Get(cs->GetScmFunction());
		if (scmFunc == nullptr)
		{
			SHOW_ERROR("Invalid Cleo Call reference. [%04X] possibly used without preceding [0AB1] in script %s\nScript suspended.", opcode, cs->GetInfoStr().c_str());
			return thread->Suspend();
		}

		// store return arguments
		static SCRIPT_VAR arguments[32];
		static bool argumentIsStr[32];
		std::forward_list<std::string> stringParams; // scope guard for strings
		if (returnArgs)
		{
			if (returnArgCount > 32)
			{
				SHOW_ERROR("Opcode [%04X] has too many (%d) args in script %s\nScript suspended.", opcode, returnArgCount, cs->GetInfoStr().c_str());
				return thread->Suspend();
			}

			auto nVarArg = GetVarArgCount(thread);
			if (returnArgCount > nVarArg)
			{
				SHOW_ERROR("Opcode [%04X] declared %d args, but %d was provided in script %s\nScript suspended.", opcode, returnArgCount, nVarArg, ((CCustomScript*)thread)->GetInfoStr().c_str());
				return thread->Suspend();
			}

			for (DWORD i = 0; i < returnArgCount; i++)
			{
				SCRIPT_VAR* arg = arguments + i;
				argumentIsStr[i] = false;

				auto paramType = (eDataType)*thread->GetBytePointer();
				if (IsImmInteger(paramType) || IsVariable(paramType))
				{
					*thread >> arg->dwParam;
				}
				else if (paramType == DT_FLOAT)
				{
					*thread >> arg->fParam;
				}
				else if (IsImmString(paramType) || IsVarString(paramType))
				{
					argumentIsStr[i] = true;

					OPCODE_READ_PARAM_STRING(str);
					stringParams.emplace_front(str);
					arg->pcParam = stringParams.front().data();
				}
				else
				{
					SHOW_ERROR("Invalid argument type '0x%02X' in opcode [%04X] in script %s\nScript suspended.", paramType, opcode, ((CCustomScript*)thread)->GetInfoStr().c_str());
					return thread->Suspend();
				}
			}
		}

		// handle program flow
		scmFunc->Return(cs); // jump back to cleo_call, right after last input param. Return slot var args starts here
		if (scmFunc->moduleExportRef != nullptr) GetInstance().ModuleSystem.ReleaseModuleRef((char*)scmFunc->moduleExportRef); // exiting export - release module
		delete scmFunc;

		if (returnArgs)
		{
			DWORD returnSlotCount = GetVarArgCount(cs);
			if (returnSlotCount > returnArgCount || (strictArgCount && returnSlotCount < returnArgCount))
			{
				SHOW_ERROR("Opcode [%04X] returned %d params, while function caller expected %d in script %s\nScript suspended.", opcode, returnArgCount, returnSlotCount, cs->GetInfoStr().c_str());
				return cs->Suspend();
			}
			else if (returnSlotCount < returnArgCount)
			{
				LOG_WARNING(thread, "Opcode [%04X] returned %d params, while function caller expected %d in script %s", opcode, returnArgCount, returnSlotCount, cs->GetInfoStr().c_str());
			}

			// set return args
			for (DWORD i = 0; i < returnArgCount; i++)
			{
				auto arg = (SCRIPT_VAR*)thread->GetBytePointer();

				auto paramType = *(eDataType*)arg;
				if (IsVarString(paramType))
				{
					OPCODE_WRITE_PARAM_STRING(arguments[i].pcParam);
				} 
				else if (IsVariable(paramType))
				{
					if (argumentIsStr[i]) // source was string, write it into provided buffer ptr
					{
						OPCODE_WRITE_PARAM_STRING(arguments[i].pcParam);
					}
					else
						*thread << arguments[i].dwParam;
				}
				else
				{
					SHOW_ERROR("Invalid output argument type '0x%02X' in opcode [%04X] in script %s\nScript suspended.", paramType, opcode, ((CCustomScript*)thread)->GetInfoStr().c_str());
					return thread->Suspend();
				}
			}
		}

		SkipUnusedVarArgs(thread); // skip var args terminator too

		return OR_CONTINUE;
	}

	inline void ThreadJump(CRunningScript *thread, int off)
	{
		thread->SetIp(off < 0 ? thread->GetBasePointer() - off : scmBlock + off);
	}

	void SkipUnusedVarArgs(CRunningScript *thread)
	{
		while (thread->PeekDataType() != DT_END)
			CLEO_SkipOpcodeParams(thread, 1);

		thread->IncPtr(); // skip terminator
	}

	DWORD GetVarArgCount(CRunningScript* thread)
	{
		const auto ip = thread->GetBytePointer();

		DWORD count = 0;
		while (thread->PeekDataType() != DT_END)
		{
			CLEO_SkipOpcodeParams(thread, 1);
			count++;
		}

		thread->SetIp(ip); // restore
		return count;
	}

	/************************************************************************/
	/*						Opcode definitions								*/
	/************************************************************************/

	OpcodeResult __stdcall CCustomOpcodeSystem::opcode_0051(CRunningScript* thread) // GOSUB return
	{
		if (thread->SP == 0 && thread->IsCustom() && !IsLegacyScript(thread)) // CLEO5 - allow use of GOSUB `return` to exit cleo calls too
		{
			return GetInstance().OpcodeSystem.CleoReturnGeneric(0x0051, thread, false); // try CLEO's function return
		}

		if (thread->SP == 0)
		{
			SHOW_ERROR("`return` used without preceding `gosub` call in script %s\nScript suspended.", ((CCustomScript*)thread)->GetInfoStr().c_str());
			return thread->Suspend();
		}

		return originalOpcodeHandlers[0x0051](thread, 0x0051); // call game's original
	}

	//0A92=-1,create_custom_thread %1d%
	OpcodeResult __stdcall opcode_0A92(CRunningScript *thread)
	{
		OPCODE_READ_PARAM_STRING(path);

		auto filename = reinterpret_cast<CCustomScript*>(thread)->ResolvePath(path, DIR_CLEO); // legacy: default search location is game\cleo directory
		TRACE("[0A92] Starting new custom script %s from thread named %s", filename.c_str(), thread->GetName().c_str());

		auto cs = new CCustomScript(filename.c_str(), false, thread);
		SetScriptCondResult(thread, cs && cs->IsOK());
		if (cs && cs->IsOK())
		{
			GetInstance().ScriptEngine.AddCustomScript(cs);
			TransmitScriptParams(thread, cs);
			cs->SetDebugMode(reinterpret_cast<CCustomScript*>(thread)->GetDebugMode());
		}
		else
		{
			if (cs) delete cs;
			SkipUnusedVarArgs(thread);
			LOG_WARNING(0, "Failed to load script '%s' in script ", filename.c_str(), ((CCustomScript*)thread)->GetInfoStr().c_str());
		}

		return OR_CONTINUE;
	}

	//0A93=0,terminate_this_custom_script
	OpcodeResult __stdcall opcode_0A93(CRunningScript *thread)
	{
		CCustomScript *cs = reinterpret_cast<CCustomScript *>(thread);
		if (thread->IsMission() || !cs->IsCustom())
		{
			LOG_WARNING(0, "Incorrect usage of opcode [0A93] in script %s", ((CCustomScript*)thread)->GetInfoStr().c_str());

			return OR_CONTINUE;
		}
		GetInstance().ScriptEngine.RemoveCustomScript(cs);
		return OR_INTERRUPT;
	}

	//0A94=-1,create_custom_mission %1d%
	OpcodeResult __stdcall opcode_0A94(CRunningScript *thread)
	{
		OPCODE_READ_PARAM_STRING(path);

		auto filename = reinterpret_cast<CCustomScript*>(thread)->ResolvePath(path, DIR_CLEO); // legacy: default search location is game\cleo directory
		filename += ".cm"; // add custom mission extension
		TRACE("[0A94] Starting new custom mission %s from thread named %s", filename.c_str(), thread->GetName().c_str());

		auto cs = new CCustomScript(filename.c_str(), true, thread);
		SetScriptCondResult(thread, cs && cs->IsOK());
		if (cs && cs->IsOK())
		{
			auto csscript = reinterpret_cast<CCustomScript*>(thread);
			if (csscript->IsCustom())
				cs->SetCompatibility(csscript->GetCompatibility());
			GetInstance().ScriptEngine.AddCustomScript(cs);
			memset(missionLocals, 0, 1024 * sizeof(SCRIPT_VAR)); // same as CTheScripts::WipeLocalVariableMemoryForMissionScript
			TransmitScriptParams(thread, (CRunningScript*)((BYTE*)missionLocals - 0x3C));
			cs->SetDebugMode(reinterpret_cast<CCustomScript*>(thread)->GetDebugMode());
		}
		else
		{
			if (cs) delete cs;
			SkipUnusedVarArgs(thread);
			LOG_WARNING(0, "[0A94] Failed to load mission '%s' from script '%s'.", filename.c_str(), thread->GetName().c_str());
		}

		return OR_CONTINUE;
	}

	//0A95=0,enable_thread_saving
	OpcodeResult __stdcall opcode_0A95(CRunningScript *thread)
	{
		reinterpret_cast<CCustomScript *>(thread)->enable_saving();
		return OR_CONTINUE;
	}

	//0AA0=1,gosub_if_false %1p%
	OpcodeResult __stdcall opcode_0AA0(CRunningScript *thread)
	{
		int off;
		*thread >> off;
		if (thread->GetConditionResult()) return OR_CONTINUE;
		thread->PushStack(thread->GetBytePointer());
		ThreadJump(thread, off);
		return OR_CONTINUE;
	}

	//0AA1=0,return_if_false
	OpcodeResult __stdcall opcode_0AA1(CRunningScript *thread)
	{
		if (thread->GetConditionResult()) return OR_CONTINUE;
		thread->SetIp(thread->PopStack());
		return OR_CONTINUE;
	}

	//0AA9=0,  is_game_version_original
	OpcodeResult __stdcall opcode_0AA9(CRunningScript *thread)
	{
		auto gv = GetInstance().VersionManager.GetGameVersion();
		auto cs = (CCustomScript*)thread;
		SetScriptCondResult(thread, gv == GV_US10 || (cs->IsCustom() && cs->GetCompatibility() <= CLEO_VER_4_MIN && gv == GV_EU10));
		return OR_CONTINUE;
	}

	//0AB0=1,  key_pressed %1d%
	OpcodeResult __stdcall opcode_0AB0(CRunningScript *thread)
	{
		DWORD key;
		*thread >> key;
		SHORT state = GetKeyState(key);
		SetScriptCondResult(thread, (GetKeyState(key) & 0x8000) != 0);
		return OR_CONTINUE;
	}

	//0AB1=-1,call_scm_func %1p%
	OpcodeResult __stdcall opcode_0AB1(CRunningScript *thread)
	{
		int label = 0;
		std::string moduleTxt;

		auto paramType = thread->PeekDataType();
		if (IsImmInteger(paramType) || IsVariable(paramType))
		{
			*thread >> label; // label offset
		}
		else if (IsImmString(paramType) || IsVarString(paramType))
		{
			char tmp[MAX_STR_LEN + 1];
			auto str = ReadStringParam(thread, tmp, sizeof(tmp)); // string with module and export name
			if (str != nullptr) moduleTxt = str;
		}
		else
		{
			SHOW_ERROR("Invalid type of first argument in opcode [0AB1], in script %s", ((CCustomScript*)thread)->GetInfoStr().c_str());
			return thread->Suspend();
		}
	
		ScmFunction* scmFunc = new ScmFunction(thread);
		
		// parse module reference text
		if (!moduleTxt.empty())
		{
			auto pos = moduleTxt.find('@');
			if (pos == moduleTxt.npos)
			{
				SHOW_ERROR("Invalid module reference '%s' in opcode [0AB1] in script %s \nScript suspended.", moduleTxt.c_str(), ((CCustomScript*)thread)->GetInfoStr().c_str());
				return thread->Suspend();
			}
			std::string_view strExport = moduleTxt.substr(0, pos);
			std::string_view strModule = moduleTxt.substr(pos + 1);

			// get module's file absolute path
			auto modulePath = std::string(strModule);
			modulePath = reinterpret_cast<CCustomScript*>(thread)->ResolvePath(modulePath.c_str(), DIR_SCRIPT); // by default search relative to current script location

			// get export reference
			auto scriptRef = GetInstance().ModuleSystem.GetExport(modulePath, strExport);
			if (!scriptRef.Valid())
			{
				SHOW_ERROR("Not found module '%s' export '%s', requested by opcode [0AB1] in script %s", modulePath.c_str(), moduleTxt.c_str(), ((CCustomScript*)thread)->GetInfoStr().c_str());
				return thread->Suspend();
			}
			scmFunc->moduleExportRef = scriptRef.base; // to be released on return

			reinterpret_cast<CCustomScript*>(thread)->SetScriptFileDir(FS::path(modulePath).parent_path().string().c_str());
			reinterpret_cast<CCustomScript*>(thread)->SetScriptFileName(FS::path(modulePath).filename().string().c_str());
			thread->SetBaseIp(scriptRef.base);
			label = scriptRef.offset;
		}

		// "number of input parameters" opcode argument
		DWORD nParams = 0;
		paramType = thread->PeekDataType();
		if (paramType != DT_END)
		{
			if (IsImmInteger(paramType))
			{
				*thread >> nParams;
			}
			else
			{
				SHOW_ERROR("Invalid type (%s) of the 'input param count' argument in opcode [0AB1] in script %s \nScript suspended.", ToKindStr(paramType), ((CCustomScript*)thread)->GetInfoStr().c_str());
				return thread->Suspend();
			}
		}
		if (nParams)
		{
			auto nVarArg = GetVarArgCount(thread);
			if (nParams > nVarArg) // if less it means there are return params too
			{
				SHOW_ERROR("Opcode [0AB1] declared %d input args, but provided %d in script %s\nScript suspended.", nParams, nVarArg, ((CCustomScript*)thread)->GetInfoStr().c_str());
				return thread->Suspend();
			}

			if (nParams > 32)
			{
				SHOW_ERROR("Argument count %d is out of supported range (32) of opcode [0AB1] in script %s", nParams, ((CCustomScript*)thread)->GetInfoStr().c_str());
				return thread->Suspend();
			}
		}

		static SCRIPT_VAR arguments[32];
		SCRIPT_VAR* locals = thread->IsMission() ? missionLocals : thread->GetVarPtr();
		SCRIPT_VAR* localsEnd = locals + 32;
		SCRIPT_VAR* storedLocals = scmFunc->savedTls;

		// collect arguments
		for (DWORD i = 0; i < min(nParams, 32); i++)
		{
			SCRIPT_VAR* arg = arguments + i;

			auto paramType = (eDataType)*thread->GetBytePointer();
			if (IsImmInteger(paramType) || IsVariable(paramType))
			{
				*thread >> arg->dwParam;
			}
			else if(paramType == DT_FLOAT)
			{
				*thread >> arg->fParam;
			}
			else if (IsVarString(paramType))
			{
				arg->pParam = GetScriptParamPointer(thread);
				if (arg->pParam >= locals && arg->pParam < localsEnd) // correct scoped variable's pointer
				{
					arg->dwParam -= (DWORD)locals;
					arg->dwParam += (DWORD)storedLocals;
				}
			}
			else if (IsImmString(paramType)) // those texts exists in script code, but without terminator character. Copy is necessary
			{
				char tmp[MAX_STR_LEN + 1];
				auto str = ReadStringParam(thread, tmp, sizeof(tmp));
				scmFunc->stringParams.emplace_back(str);
				arg->pcParam = (char*)scmFunc->stringParams.back().c_str();
			}
			else
			{
				SHOW_ERROR("Invalid argument type '0x%02X' in opcode [0AB1] in script %s\nScript suspended.", paramType, ((CCustomScript*)thread)->GetInfoStr().c_str());
				return thread->Suspend();
			}
		}

		// skip unused args
		if (nParams > 32) 
			GetScriptParams(thread, nParams - 32);

		// all arguments read
		scmFunc->retnAddress = thread->GetBytePointer();

		// pass arguments as new scope local variables
		memcpy(locals, arguments, nParams * sizeof(SCRIPT_VAR));

		// initialize rest of new scope local variables
		auto cs = reinterpret_cast<CCustomScript*>(thread);
		if (cs->IsCustom() && cs->GetCompatibility() >= CLEO_VER_4_MIN) // CLEO 3 did not initialised local variables
		{
			for (DWORD i = nParams; i < 32; i++)
			{
				cs->SetIntVar(i, 0); // fill with zeros
			}
		}

		// jump to label
		ThreadJump(thread, label); // script offset
		return OR_CONTINUE;
	}

	//0AB2=-1,cleo_return
	OpcodeResult __stdcall opcode_0AB2(CRunningScript *thread)
	{
		DWORD returnParamCount = GetVarArgCount(thread);
		if (returnParamCount)
		{
			auto paramType = (eDataType)*thread->GetBytePointer();
			if (!IsImmInteger(paramType))
			{
				SHOW_ERROR("Invalid type of first argument in opcode [0AB2], in script %s", ((CCustomScript*)thread)->GetInfoStr().c_str());
				return thread->Suspend();
			}
			DWORD declaredParamCount; *thread >> declaredParamCount;

			if (returnParamCount - 1 < declaredParamCount) // minus 'num args' itself
			{
				SHOW_ERROR("Opcode [0AB2] declared %d return args, but provided %d in script %s\nScript suspended.", declaredParamCount, returnParamCount - 1, ((CCustomScript*)thread)->GetInfoStr().c_str());
				return thread->Suspend();
			}
			else if (returnParamCount - 1 > declaredParamCount) // more args than needed, not critical
			{
				LOG_WARNING(thread, "Opcode [0AB2] declared %d return args, but provided %d in script %s", declaredParamCount, returnParamCount - 1, ((CCustomScript*)thread)->GetInfoStr().c_str());
			}

			returnParamCount = declaredParamCount;
		}

		return GetInstance().OpcodeSystem.CleoReturnGeneric(0x0AB2, thread, true, returnParamCount);
	}

	//0AB3=2,set_cleo_shared_var %1d% = %2d%
	OpcodeResult __stdcall opcode_0AB3(CRunningScript *thread)
	{
		auto varIdx = OPCODE_READ_PARAM_INT();

		const auto VarCount = _countof(CScriptEngine::CleoVariables);
		if (varIdx < 0 || varIdx >= VarCount)
		{
			SHOW_ERROR("Variable index '%d' out of supported range in script %s\nScript suspended.", varIdx, ((CCustomScript*)thread)->GetInfoStr().c_str());
			return thread->Suspend();
		}

		auto paramType = thread->PeekDataType();
		if (!IsImmInteger(paramType) &&
			!IsImmFloat(paramType) &&
			!IsVariable(paramType))
		{
			SHOW_ERROR("Invalid value type (%s) in script %s \nScript suspended.", ToKindStr(paramType), ((CCustomScript*)thread)->GetInfoStr().c_str());
			return thread->Suspend();
		}

		GetScriptParams(thread, 1);
		GetInstance().ScriptEngine.CleoVariables[varIdx].dwParam = opcodeParams[0].dwParam;
		return OR_CONTINUE;
	}

	//0AB4=2,%2d% = get_cleo_shared_var %1d%
	OpcodeResult __stdcall opcode_0AB4(CRunningScript *thread)
	{
		auto varIdx = OPCODE_READ_PARAM_INT();

		const auto VarCount = _countof(CScriptEngine::CleoVariables);
		if (varIdx < 0 || varIdx >= VarCount)
		{
			SHOW_ERROR("Variable index '%d' out of supported range in script %s\nScript suspended.", varIdx, ((CCustomScript*)thread)->GetInfoStr().c_str());
			return thread->Suspend();
		}

		auto paramType = thread->PeekDataType();
		if (!IsVariable(paramType))
		{
			SHOW_ERROR("Invalid result argument type (%s) in script %s \nScript suspended.", ToKindStr(paramType), ((CCustomScript*)thread)->GetInfoStr().c_str());
			return thread->Suspend();
		}

		opcodeParams[0].dwParam = GetInstance().ScriptEngine.CleoVariables[varIdx].dwParam;
		CLEO_RecordOpcodeParams(thread, 1);
		return OR_CONTINUE;
	}

	//0AB5=3,store_actor %1d% closest_vehicle_to %2d% closest_ped_to %3d%
	OpcodeResult __stdcall opcode_0AB5(CRunningScript *thread)
	{
		auto handle = OPCODE_READ_PARAM_PED_HANDLE();

		auto pPlayerPed = CPools::GetPed(handle);

		CPedIntelligence * pedintel;
		if (pPlayerPed && (pedintel = pPlayerPed->m_pIntelligence))
		{
			CVehicle * pVehicle = nullptr;
			for (int i = 0; i < NUM_SCAN_ENTITIES; i++)
			{
				pVehicle = (CVehicle*)pedintel->m_vehicleScanner.m_apEntities[i];
				if (pVehicle && pVehicle->m_nCreatedBy != 2 && !pVehicle->m_nVehicleFlags.bFadeOut)
					break;
				pVehicle = nullptr;
			}

			CPed * pPed = nullptr;
			for (int i = 0; i < NUM_SCAN_ENTITIES; i++)
			{
				pPed = (CPed*)pedintel->m_pedScanner.m_apEntities[i];
				if (pPed && pPed != pPlayerPed && (pPed->m_nCreatedBy & 0xFF) == 1 && !pPed->m_nPedFlags.bFadeOut)
					break;
				pPed = nullptr;
			}

			*thread << (pVehicle ? CPools::GetVehicleRef(pVehicle) : -1) << (pPed ? CPools::GetPedRef(pPed) : -1);
		}
		else *thread << -1 << -1;
		return OR_CONTINUE;
	}

	//0AB6=3,store_target_marker_coords_to %1d% %2d% %3d% // IF and SET
	OpcodeResult __stdcall opcode_0AB6(CRunningScript *thread)
	{
		// steam offset is different, so get it manually for now
		CGameVersionManager& gvm = GetInstance().VersionManager;
		DWORD hMarker = gvm.GetGameVersion() !=  GV_STEAM ? MenuManager->m_nTargetBlipIndex : *((DWORD*)0xC3312C);
		CMarker *pMarker;
		if (hMarker && (pMarker = &RadarBlips[LOWORD(hMarker)]) && /*pMarker->m_nPoolIndex == HIWORD(hMarker) && */pMarker->m_nBlipDisplay)
		{
			CVector coords(pMarker->m_vecPos);
			coords.z = FindGroundZ(coords.x, coords.y);

			OPCODE_WRITE_PARAM_FLOAT(coords.x);
			OPCODE_WRITE_PARAM_FLOAT(coords.y);
			OPCODE_WRITE_PARAM_FLOAT(coords.z);
			OPCODE_CONDITION_RESULT(true);
		}
		else
		{
			OPCODE_SKIP_PARAMS(3);
			OPCODE_CONDITION_RESULT(false);
		}

		return OR_CONTINUE;
	}

	//0AB7=2,get_vehicle %1d% number_of_gears_to %2d%
	OpcodeResult __stdcall opcode_0AB7(CRunningScript *thread)
	{
		auto handle = OPCODE_READ_PARAM_VEHICLE_HANDLE();

		auto vehicle = CPools::GetVehicle(handle);
		auto gears = vehicle->m_pHandlingData->m_transmissionData.m_nNumberOfGears;

		OPCODE_WRITE_PARAM_INT(gears);
		return OR_CONTINUE;
	}

	//0AB8=2,get_vehicle %1d% current_gear_to %2d%
	OpcodeResult __stdcall opcode_0AB8(CRunningScript *thread)
	{
		auto handle = OPCODE_READ_PARAM_VEHICLE_HANDLE();

		auto vehicle = CPools::GetVehicle(handle);
		auto gear = vehicle->m_nCurrentGear;

		OPCODE_WRITE_PARAM_INT(gear);
		return OR_CONTINUE;
	}

	//0ABA=1,end_custom_thread_named %1d%
	OpcodeResult __stdcall opcode_0ABA(CRunningScript *thread)
	{
		OPCODE_READ_PARAM_STRING(threadName);

		auto deleted_thread = (CCustomScript*)GetInstance().ScriptEngine.FindScriptNamed(threadName, false, true, 0);
		if (deleted_thread)
		{
			GetInstance().ScriptEngine.RemoveCustomScript(deleted_thread);
		}
		return deleted_thread == thread ? OR_INTERRUPT : OR_CONTINUE;
	}

	//0ABD=1,  vehicle %1d% siren_on
	OpcodeResult __stdcall opcode_0ABD(CRunningScript *thread)
	{
		auto handle = OPCODE_READ_PARAM_VEHICLE_HANDLE();

		auto vehicle = CPools::GetVehicle(handle);
		auto state = vehicle->m_nVehicleFlags.bSirenOrAlarm;

		OPCODE_CONDITION_RESULT(state);
		return OR_CONTINUE;
	}

	//0ABE=1,  vehicle %1d% engine_on
	OpcodeResult __stdcall opcode_0ABE(CRunningScript *thread)
	{
		auto handle = OPCODE_READ_PARAM_VEHICLE_HANDLE();

		auto vehicle = CPools::GetVehicle(handle);
		auto state = vehicle->m_nVehicleFlags.bEngineOn;

		OPCODE_CONDITION_RESULT(state);
		return OR_CONTINUE;
	}

	//0ABF=2,set_vehicle %1d% engine_state_to %2d%
	OpcodeResult __stdcall opcode_0ABF(CRunningScript *thread)
	{
		auto handle = OPCODE_READ_PARAM_VEHICLE_HANDLE();
		auto state = OPCODE_READ_PARAM_BOOL();

		auto vehicle = CPools::GetVehicle(handle);

		vehicle->m_nVehicleFlags.bEngineOn = state != false;
		return OR_CONTINUE;
	}

	//0ACA=1,show_text_box %1d%
	OpcodeResult __stdcall opcode_0ACA(CRunningScript *thread)
	{
		OPCODE_READ_PARAM_STRING(text);
		PrintHelp(text);
		return OR_CONTINUE;
	}

	//0ACB=3,show_styled_text %1d% time %2d% style %3d%
	OpcodeResult __stdcall opcode_0ACB(CRunningScript *thread)
	{
		OPCODE_READ_PARAM_STRING(text);
		auto time = OPCODE_READ_PARAM_INT();
		auto style = OPCODE_READ_PARAM_INT();

		PrintBig(text, time, style);
		return OR_CONTINUE;
	}

	//0ACC=2,show_text_lowpriority %1d% time %2d%
	OpcodeResult __stdcall opcode_0ACC(CRunningScript *thread)
	{
		OPCODE_READ_PARAM_STRING(text);
		auto time = OPCODE_READ_PARAM_INT();

		Print(text, time);
		return OR_CONTINUE;
	}

	//0ACD=2,show_text_highpriority %1d% time %2d%
	OpcodeResult __stdcall opcode_0ACD(CRunningScript *thread)
	{
		OPCODE_READ_PARAM_STRING(text);
		auto time = OPCODE_READ_PARAM_INT();

		PrintNow(text, time);
		return OR_CONTINUE;
	}

	//0ACE=-1,show_formatted_text_box %1d%
	OpcodeResult __stdcall opcode_0ACE(CRunningScript *thread)
	{
		OPCODE_READ_PARAM_STRING(format);
		char text[MAX_STR_LEN]; OPCODE_READ_FORMATTED_STRING(thread, text, sizeof(text), format)

		PrintHelp(text);
		return OR_CONTINUE;
	}

	//0ACF=-1,show_formatted_styled_text %1d% time %2d% style %3d%
	OpcodeResult __stdcall opcode_0ACF(CRunningScript *thread)
	{
		OPCODE_READ_PARAM_STRING(format);
		auto time = OPCODE_READ_PARAM_INT();
		auto style = OPCODE_READ_PARAM_INT();
		char text[MAX_STR_LEN]; OPCODE_READ_FORMATTED_STRING(thread, text, sizeof(text), format)

		PrintBig(text, time, style);
		return OR_CONTINUE;
	}

	//0AD0=-1,show_formatted_text_lowpriority %1d% time %2d%
	OpcodeResult __stdcall opcode_0AD0(CRunningScript *thread)
	{
		OPCODE_READ_PARAM_STRING(format);
		auto time = OPCODE_READ_PARAM_INT();
		char text[MAX_STR_LEN]; OPCODE_READ_FORMATTED_STRING(thread, text, sizeof(text), format)

		Print(text, time);
		return OR_CONTINUE;
	}

	//0AD1=-1,show_formatted_text_highpriority %1d% time %2d%
	OpcodeResult __stdcall opcode_0AD1(CRunningScript *thread)
	{
		OPCODE_READ_PARAM_STRING(format);
		auto time = OPCODE_READ_PARAM_INT();
		char text[MAX_STR_LEN]; OPCODE_READ_FORMATTED_STRING(thread, text, sizeof(text), format)

		PrintNow(text, time);
		return OR_CONTINUE;
	}

	//0AD2=2,  %2d% = player %1d% targeted_actor //IF and SET
	OpcodeResult __stdcall opcode_0AD2(CRunningScript *thread)
	{
		auto playerId = OPCODE_READ_PARAM_INT();

		auto pPlayerPed = GetPlayerPed(playerId); // TODO: use plugin SDK instead
		auto pTargetEntity = GetWeaponTarget(pPlayerPed);

		if (!pTargetEntity) pTargetEntity = (CEntity*)pPlayerPed->m_pPlayerTargettedPed;
		if (pTargetEntity && pTargetEntity->m_nType == ENTITY_TYPE_PED)
		{
			auto handle = CPools::GetPedRef(reinterpret_cast<CPed*>(pTargetEntity));
			OPCODE_WRITE_PARAM_INT(handle);
			OPCODE_CONDITION_RESULT(true);
		}
		else
		{
			OPCODE_WRITE_PARAM_INT(-1);
			OPCODE_CONDITION_RESULT(false);
		}
		return OR_CONTINUE;
	}

	//0AD3=-1,string %1d% format %2d% ...
	OpcodeResult __stdcall opcode_0AD3(CRunningScript *thread)
	{
		auto resultArg = GetStringParamWriteBuffer(thread); OPCODE_VALIDATE_STR_ARG_WRITE(resultArg.data)
		OPCODE_READ_PARAM_STRING(format);
		char text[MAX_STR_LEN]; OPCODE_READ_FORMATTED_STRING(thread, text, sizeof(text), format)

		WriteStringParam(resultArg, text);
		return OR_CONTINUE;
	}

	//0AD4=-1,%3d% = scan_string %1d% format %2d%  //IF and SET
	OpcodeResult __stdcall opcode_0AD4(CRunningScript *thread)
	{
		OPCODE_READ_PARAM_STRING(src);
		OPCODE_READ_PARAM_STRING(format);

		auto resultType = thread->PeekDataType();
		if (!IsVariable(resultType) && IsVarString(resultType))
		{
			SHOW_ERROR("Result parameter must be variable type, received '%s' in script %s \nScript suspended.", ToKindStr(resultType), ((CCustomScript*)thread)->GetInfoStr().c_str());
			return thread->Suspend();
		}
		int *result = (int *)GetScriptParamPointer(thread);

		// read extra params
		size_t cExParams = 0;
		SCRIPT_VAR *ExParams[35];
		for (int i = 0; i < 35; i++)
		{
			auto paramType = thread->PeekDataType();
			if (paramType != DT_END)
			{
				ExParams[i] = GetScriptParamPointer(thread);
				cExParams++;
			}
			else ExParams[i] = nullptr; // clear unused args
		}
		SkipUnusedVarArgs(thread); // and var args terminator

		*result = sscanf(src, format,
						 /* extra parameters (will be aligned automatically, but the limit of 35 elements maximum exists) */
						 ExParams[0], ExParams[1], ExParams[2], ExParams[3], ExParams[4], ExParams[5],
						 ExParams[6], ExParams[7], ExParams[8], ExParams[9], ExParams[10], ExParams[11],
						 ExParams[12], ExParams[13], ExParams[14], ExParams[15], ExParams[16], ExParams[17],
						 ExParams[18], ExParams[19], ExParams[20], ExParams[21], ExParams[22], ExParams[23],
						 ExParams[24], ExParams[25], ExParams[26], ExParams[27], ExParams[28], ExParams[29],
						 ExParams[30], ExParams[31], ExParams[32], ExParams[33], ExParams[34]);

		SetScriptCondResult(thread, cExParams == *result);
		return OR_CONTINUE;
	}

	//0ADB=2,%2d% = car_model %1d% name
	OpcodeResult __stdcall opcode_0ADB(CRunningScript *thread)
	{
		DWORD modelIndex; *thread >> modelIndex;

		CVehicleModelInfo* model;
		// if 1.0 US, prefer GetModelInfo function  makes it compatible with fastman92's limit adjuster
		if (CLEO::GetInstance().VersionManager.GetGameVersion() == CLEO::GV_US10)
			model = plugin::CallAndReturn<CVehicleModelInfo *, 0x403DA0, int>(modelIndex);
		else
			model = reinterpret_cast<CVehicleModelInfo*>(Models[modelIndex]);

		auto str = std::string(std::string_view(model->m_szGameName, sizeof(model->m_szGameName))); // to proper cstr
		WriteStringParam(thread, str.c_str());
		return OR_CONTINUE;
	}

	//0ADC=1, test_cheat %1d%
	OpcodeResult __stdcall opcode_0ADC(CRunningScript *thread)
	{
		OPCODE_READ_PARAM_STRING(text);
		SetScriptCondResult(thread, TestCheat(text));
		return OR_CONTINUE;
	}

	//0ADD=1,spawn_car_with_model %1o% at_player_location 
	OpcodeResult __stdcall opcode_0ADD(CRunningScript *thread)
	{
		DWORD mi;
		*thread >> mi;

		CVehicleModelInfo* model;
		// if 1.0 US, prefer GetModelInfo function  makes it compatible with fastman92's limit adjuster
		if (CLEO::GetInstance().VersionManager.GetGameVersion() == CLEO::GV_US10) {
			model = plugin::CallAndReturn<CVehicleModelInfo *, 0x403DA0, int>(mi);
		}
		else {
			model = reinterpret_cast<CVehicleModelInfo*>(Models[mi]);
		}
		if (model->m_nVehicleType != VEHICLE_TYPE_TRAIN && model->m_nVehicleType != VEHICLE_TYPE_UNKNOWN) SpawnCar(mi);
		return OR_CONTINUE;
	}

	//0ADE=2,%2d% = text_by_GXT_entry %1d%
	OpcodeResult __stdcall opcode_0ADE(CRunningScript *thread)
	{
		OPCODE_READ_PARAM_STRING_LEN(gxt, 7); // GXT labels can be max 7 character long

		auto txt = GetInstance().TextManager.Get(gxt);

		if (IsVarString(thread->PeekDataType()))
		{
			OPCODE_WRITE_PARAM_STRING(txt);
		}
		else
		{
			OPCODE_WRITE_PARAM_PTR(txt); // address of the text
		}
		return OR_CONTINUE;
	}

	//0ADF=2,add_dynamic_GXT_entry %1d% text %2d%
	OpcodeResult __stdcall opcode_0ADF(CRunningScript *thread)
	{
		OPCODE_READ_PARAM_STRING_LEN(gxt, 7); // GXT labels can be max 7 character long
		OPCODE_READ_PARAM_STRING(txt);

		GetInstance().TextManager.AddFxt(gxt, txt);
		return OR_CONTINUE;
	}

	//0AE0=1,remove_dynamic_GXT_entry %1d%
	OpcodeResult __stdcall opcode_0AE0(CRunningScript *thread)
	{
		OPCODE_READ_PARAM_STRING_LEN(gxt, 7); // GXT labels can be max 7 character long

		GetInstance().TextManager.RemoveFxt(gxt);
		return OR_CONTINUE;
	}

	//0AE1=7,%7d% = find_actor_near_point %1d% %2d% %3d% in_radius %4d% find_next %5h% pass_deads %6h% //IF and SET
	OpcodeResult __stdcall opcode_0AE1(CRunningScript *thread)
	{
		CVector center = {};
		center.x = OPCODE_READ_PARAM_FLOAT();
		center.y = OPCODE_READ_PARAM_FLOAT();
		center.z = OPCODE_READ_PARAM_FLOAT();
		auto radius = OPCODE_READ_PARAM_FLOAT();
		auto findNext = OPCODE_READ_PARAM_BOOL();
		auto passDead = OPCODE_READ_PARAM_INT();

		static DWORD stat_last_found = 0;
		auto& pool = *CPools::ms_pPedPool;

		DWORD& last_found = reinterpret_cast<CCustomScript *>(thread)->IsCustom() ?
			reinterpret_cast<CCustomScript *>(thread)->GetLastSearchPed() :
			stat_last_found;

		if (!findNext) last_found = 0;

		for (int index = last_found; index < pool.m_nSize; ++index)
		{
			if (auto obj = pool.GetAt(index))
			{
				if (passDead != -1 && (obj->IsPlayer() || (passDead && !IsAvailable(obj))/* || obj->GetOwner() == 2*/ || obj->m_nPedFlags.bFadeOut))
					continue;

				if (radius >= 1000.0f || (VectorSqrMagnitude(obj->GetPosition() - center) <= radius * radius))
				{
					last_found = index + 1;	// on next opcode call start search from next index
											//if(last_found >= (unsigned)pool.GetSize()) last_found = 0;
											//obj->PedCreatedBy = 2; // add reference to found actor

					auto found = pool.GetRef(obj);
					OPCODE_WRITE_PARAM_INT(found);
					OPCODE_CONDITION_RESULT(true);
					return OR_CONTINUE;
				}
			}
		}

		last_found = 0;
		OPCODE_WRITE_PARAM_INT(-1);
		OPCODE_CONDITION_RESULT(false);
		return OR_CONTINUE;
	}

	//0AE2=7,%7d% = find_vehicle_near_point %1d% %2d% %3d% in_radius %4d% find_next %5h% pass_wrecked %6h% //IF and SET
	OpcodeResult __stdcall opcode_0AE2(CRunningScript *thread)
	{
		CVector center = {};
		center.x = OPCODE_READ_PARAM_FLOAT();
		center.y = OPCODE_READ_PARAM_FLOAT();
		center.z = OPCODE_READ_PARAM_FLOAT();
		auto radius = OPCODE_READ_PARAM_FLOAT();
		auto findNext = OPCODE_READ_PARAM_BOOL();
		auto passWreck = OPCODE_READ_PARAM_INT();

		static DWORD stat_last_found = 0;
		auto& pool = *CPools::ms_pVehiclePool;

		DWORD& last_found = reinterpret_cast<CCustomScript*>(thread)->IsCustom() ?
			reinterpret_cast<CCustomScript *>(thread)->GetLastSearchVehicle() :
			stat_last_found;

		if (!findNext) last_found = 0;

		for (int index = last_found; index < pool.m_nSize; ++index)
		{
			if (auto obj = pool.GetAt(index))
			{
				if ((passWreck && IsWrecked(obj)) || (/*obj->GetOwner() == 2 ||*/ obj->m_nVehicleFlags.bFadeOut))
					continue;

				if (radius >= 1000.0f || (VectorSqrMagnitude(obj->GetPosition() - center) <= radius * radius))
				{
					last_found = index + 1;	// on next opcode call start search from next index
											//if(last_found >= (unsigned)pool.GetSize()) last_found = 0;
											// obj.referenceType = 2; // add reference to found actor

					auto found = pool.GetRef(obj);
					OPCODE_WRITE_PARAM_INT(found);
					OPCODE_CONDITION_RESULT(true);
					return OR_CONTINUE;
				}
			}
		}

		last_found = 0;
		OPCODE_WRITE_PARAM_INT(-1);
		OPCODE_CONDITION_RESULT(false);
		return OR_CONTINUE;
	}

	//0AE3=6,%6d% = find_object_near_point %1d% %2d% %3d% in_radius %4d% find_next %5h% //IF and SET
	OpcodeResult __stdcall opcode_0AE3(CRunningScript *thread)
	{
		CVector center = {};
		center.x = OPCODE_READ_PARAM_FLOAT();
		center.y = OPCODE_READ_PARAM_FLOAT();
		center.z = OPCODE_READ_PARAM_FLOAT();
		auto radius = OPCODE_READ_PARAM_FLOAT();
		auto findNext = OPCODE_READ_PARAM_BOOL();

		static DWORD stat_last_found = 0;
		auto& pool = *CPools::ms_pObjectPool;

		auto cs = reinterpret_cast<CCustomScript *>(thread);
		DWORD& last_found = cs->IsCustom() ? cs->GetLastSearchObject() : stat_last_found;

		if (!findNext) last_found = 0;

		for (int index = last_found; index < pool.m_nSize; ++index)
		{
			if (auto obj = pool.GetAt(index))
			{
				if (obj->m_nObjectFlags.bFadingIn) continue; // this is actually .bFadingOut (yet?)

				if (radius >= 1000.0f || (VectorSqrMagnitude(obj->GetPosition() - center) <= radius * radius))
				{
					last_found = index + 1;	// on next opcode call start search from next index
											//if(last_found >= (unsigned)pool.GetSize()) last_found = 0;
											// obj.referenceType = 2; // add reference to found actor

					auto found = pool.GetRef(obj);
					OPCODE_WRITE_PARAM_INT(found);
					OPCODE_CONDITION_RESULT(true);
					return OR_CONTINUE;
				}
			}
		}

		last_found = 0;
		OPCODE_WRITE_PARAM_INT(-1);
		OPCODE_CONDITION_RESULT(false);
		return OR_CONTINUE;
	}

	//0AED=3,%3d% = float %1d% to_string_format %2d%
	OpcodeResult __stdcall opcode_0AED(CRunningScript *thread)
	{
		// this opcode is useless now
		auto val = OPCODE_READ_PARAM_FLOAT();
		OPCODE_READ_PARAM_STRING(format);
		auto resultArg = GetStringParamWriteBuffer(thread); OPCODE_VALIDATE_STR_ARG_WRITE(resultArg.data)

		sprintf_s(resultArg.data, resultArg.size, format, val);
		return OR_CONTINUE;
	}

	//0AEE=3,%3d% = %1d% exp %2d% //all floats
	OpcodeResult __stdcall opcode_0AEE(CRunningScript *thread)
	{
		float base, arg;
		*thread >> base >> arg;
		*thread << (float)pow(base, arg);
		return OR_CONTINUE;
	}

	//0AEF=3,%3d% = log %1d% base %2d% //all floats
	OpcodeResult __stdcall opcode_0AEF(CRunningScript *thread)
	{
		float base, arg;
		*thread >> arg >> base;
		*thread << (float)(log(arg) / log(base));
		return OR_CONTINUE;
	}

	//0DD5=1,%1d% = get_platform
	OpcodeResult __stdcall opcode_0DD5(CRunningScript* thread)
	{
		*thread << PLATFORM_WINDOWS;
		return OR_CONTINUE;
	}

	//2002=-1, cleo_return_with ...
	OpcodeResult __stdcall opcode_2002(CRunningScript* thread)
	{
		DWORD argCount = GetVarArgCount(thread);
		if (argCount < 1)
		{
			SHOW_ERROR("Opcode [2002] missing condition result argument in script %s\nScript suspended.", ((CCustomScript*)thread)->GetInfoStr().c_str());
			return thread->Suspend();
		}

		DWORD result; *thread >> result;
		argCount--;
		SetScriptCondResult(thread, result != 0);

		return GetInstance().OpcodeSystem.CleoReturnGeneric(0x2002, thread, true, argCount);
	}

	//2003=-1, cleo_return_fail
	OpcodeResult __stdcall opcode_2003(CRunningScript* thread)
	{
		DWORD argCount = GetVarArgCount(thread);
		if (argCount != 0) // argument(s) not supported yet
		{
			SHOW_ERROR("Too many arguments of opcode [2003] in script %s\nScript suspended.", ((CCustomScript*)thread)->GetInfoStr().c_str());
			return thread->Suspend();
		}

		SetScriptCondResult(thread, false);
		return GetInstance().OpcodeSystem.CleoReturnGeneric(0x2003, thread);
	}
}



/********************************************************************/

// API
extern "C"
{
	using namespace CLEO;
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4550)
#endif

	BOOL WINAPI CLEO_RegisterOpcode(WORD opcode, CustomOpcodeHandler callback)
	{
		return CCustomOpcodeSystem::RegisterOpcode(opcode, callback);
	}

	BOOL WINAPI CLEO_RegisterCommand(const char* commandName, CustomOpcodeHandler callback)
	{
		WORD opcode = GetInstance().OpcodeInfoDb.GetOpcode(commandName);
		if (opcode == 0xFFFF)
		{
			LOG_WARNING(0, "Failed to register opcode [%s]! Command name not found in the database.", commandName);
			return false;
		}

		return CCustomOpcodeSystem::RegisterOpcode(opcode, callback);
	}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

	DWORD WINAPI CLEO_GetIntOpcodeParam(CLEO::CRunningScript* thread)
	{
		DWORD result;
		*thread >> result;
		return result;
	}

	float WINAPI CLEO_GetFloatOpcodeParam(CLEO::CRunningScript* thread)
	{
		float result;
		*thread >> result;
		return result;
	}

	void WINAPI CLEO_SetIntOpcodeParam(CLEO::CRunningScript* thread, DWORD value)
	{
		*thread << value;
	}

	void WINAPI CLEO_SetFloatOpcodeParam(CLEO::CRunningScript* thread, float value)
	{
		*thread << value;
	}

	LPCSTR WINAPI CLEO_ReadStringOpcodeParam(CLEO::CRunningScript* thread, char* buff, int buffSize)
	{
		static char internal_buff[MAX_STR_LEN + 1]; // and terminator
		if (!buff) 
		{
			buff = internal_buff;
			buffSize = (buffSize > 0) ? min(buffSize, sizeof(internal_buff)) : sizeof(internal_buff); // allow user's length limit
		}

		auto result = ReadStringParam(thread, buff, buffSize);
		return (result != nullptr) ? buff : nullptr;
	}

	LPCSTR WINAPI CLEO_ReadStringPointerOpcodeParam(CLEO::CRunningScript* thread, char* buff, int buffSize)
	{
		static char internal_buff[MAX_STR_LEN + 1]; // and terminator
		bool userBuffer = buff != nullptr;
		if (!userBuffer)
		{
			buff = internal_buff;
			buffSize = (buffSize > 0) ? min(buffSize, sizeof(internal_buff)) : sizeof(internal_buff); // allow user's length limit
		}

		return ReadStringParam(thread, buff, buffSize);
	}

	void WINAPI CLEO_ReadStringParamWriteBuffer(CLEO::CRunningScript* thread, char** outBuf, int* outBufSize, DWORD* outNeedsTerminator)
	{
		if (thread == nullptr || 
			outBuf == nullptr || 
			outBufSize == nullptr ||
			outNeedsTerminator == nullptr)
		{
			LOG_WARNING(thread, "Invalid argument of CLEO_ReadStringParamWriteBuffer in script %s", ((CCustomScript*)thread)->GetInfoStr().c_str());
			return;
		}

		auto target = GetStringParamWriteBuffer(thread);
		*outBuf = target.data;
		*outBufSize = target.size;
		*outNeedsTerminator = target.needTerminator;
	}

	DWORD WINAPI CLEO_PeekIntOpcodeParam(CLEO::CRunningScript* thread)
	{
		// store state
		auto param = opcodeParams[0];
		auto ip = thread->CurrentIP;
		auto count = GetInstance().OpcodeSystem.handledParamCount;

		GetScriptParams(thread, 1);
		DWORD result = opcodeParams[0].dwParam;

		// restore state
		thread->CurrentIP = ip;
		GetInstance().OpcodeSystem.handledParamCount = count;
		opcodeParams[0] = param;

		return result;
	}

	float WINAPI CLEO_PeekFloatOpcodeParam(CLEO::CRunningScript* thread)
	{
		// store state
		auto param = opcodeParams[0];
		auto ip = thread->CurrentIP;
		auto count = GetInstance().OpcodeSystem.handledParamCount;

		GetScriptParams(thread, 1);
		float result = opcodeParams[0].fParam;

		// restore state
		thread->CurrentIP = ip;
		GetInstance().OpcodeSystem.handledParamCount = count;
		opcodeParams[0] = param;

		return result;
	}

	SCRIPT_VAR* WINAPI CLEO_PeekPointerToScriptVariable(CLEO::CRunningScript* thread)
	{
		// store state
		auto ip = thread->CurrentIP;
		auto count = GetInstance().OpcodeSystem.handledParamCount;

		auto result = GetScriptParamPointer(thread);

		// restore state
		thread->CurrentIP = ip;
		GetInstance().OpcodeSystem.handledParamCount = count;

		return result;
	}

	SCRIPT_VAR* WINAPI CLEO_GetOpcodeParamsArray()
	{
		return opcodeParams;
	}

	DWORD WINAPI CLEO_GetParamsHandledCount()
	{
		return GetInstance().OpcodeSystem.handledParamCount;
	}

	void WINAPI CLEO_WriteStringOpcodeParam(CLEO::CRunningScript* thread, const char* str)
	{
		if(!WriteStringParam(thread, str))
			LOG_WARNING(thread, "%s in script %s", CCustomOpcodeSystem::lastErrorMsg.c_str(), ((CCustomScript*)thread)->GetInfoStr().c_str());
	}

	char* WINAPI CLEO_ReadParamsFormatted(CLEO::CRunningScript* thread, const char* format, char* buf, int bufSize)
	{
		static char internal_buf[MAX_STR_LEN * 4];
		if (!buf) { buf = internal_buf; bufSize = sizeof(internal_buf); }
		if (!bufSize) bufSize = MAX_STR_LEN;

		if(ReadFormattedString(thread, buf, bufSize, format) == -1) // error?
		{
			LOG_WARNING(thread, "%s in script %s", CCustomOpcodeSystem::lastErrorMsg.c_str(), ((CCustomScript*)thread)->GetInfoStr().c_str());
			return nullptr; // error
		}

		return buf;
	}

	void WINAPI CLEO_SetThreadCondResult(CLEO::CRunningScript* thread, BOOL result)
	{
		SetScriptCondResult(thread, result != FALSE);
	}

	DWORD WINAPI CLEO_GetVarArgCount(CLEO::CRunningScript* thread)
	{
		return GetVarArgCount(thread);
	}

	void WINAPI CLEO_SkipOpcodeParams(CLEO::CRunningScript* thread, int count)
	{
		for (int i = 0; i < count; i++)
		{
			switch (thread->ReadDataType())
			{
			case DT_VAR:
			case DT_LVAR:
			case DT_VAR_STRING:
			case DT_LVAR_STRING:
			case DT_VAR_TEXTLABEL:
			case DT_LVAR_TEXTLABEL:
				thread->IncPtr(2);
				break;
			case DT_VAR_ARRAY:
			case DT_LVAR_ARRAY:
			case DT_VAR_TEXTLABEL_ARRAY:
			case DT_LVAR_TEXTLABEL_ARRAY:
			case DT_VAR_STRING_ARRAY:
			case DT_LVAR_STRING_ARRAY:
				thread->IncPtr(6);
				break;
			case DT_BYTE:
			//case DT_END: // should be only skipped with var args dediacated functions
				thread->IncPtr();
				break;
			case DT_WORD:
				thread->IncPtr(2);
				break;
			case DT_DWORD:
			case DT_FLOAT:
				thread->IncPtr(4);
				break;
			case DT_VARLEN_STRING:
				thread->IncPtr((int)1 + *thread->GetBytePointer()); // as unsigned! length byte + string data
				break;

			case DT_TEXTLABEL:
				thread->IncPtr(8);
				break;
			case DT_STRING:
				thread->IncPtr(16);
				break;
			}
		}

		GetInstance().OpcodeSystem.handledParamCount += count;
	}

	void WINAPI CLEO_SkipUnusedVarArgs(CLEO::CRunningScript* thread)
	{
		SkipUnusedVarArgs(thread);
	}

	void WINAPI CLEO_ThreadJumpAtLabelPtr(CLEO::CRunningScript* thread, int labelPtr)
	{
		ThreadJump(thread, labelPtr);
	}

	int WINAPI CLEO_GetOperandType(const CLEO::CRunningScript* thread)
	{
		return (int)thread->PeekDataType();
	}

	void WINAPI CLEO_RetrieveOpcodeParams(CLEO::CRunningScript *thread, int count)
	{
		GetScriptParams(thread, count);
	}

	void WINAPI CLEO_RecordOpcodeParams(CLEO::CRunningScript *thread, int count)
	{
		SetScriptParams(thread, count);
	}

	SCRIPT_VAR * WINAPI CLEO_GetPointerToScriptVariable(CLEO::CRunningScript* thread)
	{
		return GetScriptParamPointer(thread);
	}

	RwTexture * WINAPI CLEO_GetScriptTextureById(CLEO::CRunningScript* thread, int id)
	{
		CCustomScript* customScript = reinterpret_cast<CCustomScript*>(thread);
		// We need to store-restore to update the texture list, not optimized, but this will not be used every frame anyway
		customScript->StoreScriptTextures();
		RwTexture *texture = customScript->GetScriptTextureById(id - 1);
		customScript->RestoreScriptTextures();
		return texture;
	}

	DWORD WINAPI CLEO_GetInternalAudioStream(CLEO::CRunningScript* thread, DWORD stream) // arg CAudioStream *
	{
		return stream; // CAudioStream::streamInternal offset is 0
	}

	CLEO::CRunningScript* WINAPI CLEO_CreateCustomScript(CLEO::CRunningScript* fromThread, const char *script_name, int label)
	{
		auto filename = reinterpret_cast<CCustomScript*>(fromThread)->ResolvePath(script_name, DIR_CLEO); // legacy: default search location is game\cleo directory

		if (label != 0) // create from label
		{
			TRACE("Starting new custom script from thread named %s label %i", filename.c_str(), label);
		}
		else
		{
			TRACE("Starting new custom script %s", filename.c_str());
		}

		// if "label == 0" then "script_name" need to be the file name
		auto cs = new CCustomScript(filename.c_str(), false, fromThread, label);
		if (fromThread) SetScriptCondResult(fromThread, cs && cs->IsOK());
		if (cs && cs->IsOK())
		{
			GetInstance().ScriptEngine.AddCustomScript(cs);
			if (fromThread) TransmitScriptParams(fromThread, cs);

			cs->SetDebugMode(fromThread ? 
				reinterpret_cast<CCustomScript*>(fromThread)->GetDebugMode() : // from parent
				GetInstance().ScriptEngine.NativeScriptsDebugMode); // global
		}
		else
		{
			if (cs) delete cs;
			if (fromThread) SkipUnusedVarArgs(fromThread);
			LOG_WARNING(0, "Failed to load script '%s'.", filename.c_str());
			return nullptr;
		}

		return cs;
	}

	CLEO::CRunningScript* WINAPI CLEO_GetLastCreatedCustomScript()
	{
		return GetInstance().ScriptEngine.LastScriptCreated;
	}

	CLEO::CRunningScript* WINAPI CLEO_GetScriptByName(const char* threadName, BOOL standardScripts, BOOL customScripts, DWORD resultIndex)
	{
		return GetInstance().ScriptEngine.FindScriptNamed(threadName, standardScripts, customScripts, resultIndex);
	}

	CLEO::CRunningScript* WINAPI CLEO_GetScriptByFilename(const char* path, DWORD resultIndex)
	{
		return GetInstance().ScriptEngine.FindScriptByFilename(path, resultIndex);
	}

	void WINAPI CLEO_AddScriptDeleteDelegate(FuncScriptDeleteDelegateT func)
	{
		scriptDeleteDelegate += func;
	}

	void WINAPI CLEO_RemoveScriptDeleteDelegate(FuncScriptDeleteDelegateT func)
	{
		scriptDeleteDelegate -= func;
	}

	void WINAPI CLEO_ResolvePath(CLEO::CRunningScript* thread, char* inOutPath, DWORD pathMaxLen)
	{
		if (thread == nullptr || inOutPath == nullptr || pathMaxLen < 2)
		{
			return; // invalid param
		}

		auto resolved = reinterpret_cast<CCustomScript*>(thread)->ResolvePath(inOutPath);
		
		if (resolved.length() >= pathMaxLen)
			resolved.resize(pathMaxLen - 1); // and terminator character

		std::memcpy(inOutPath, resolved.c_str(), resolved.length() + 1); // with terminator
	}

	BOOL WINAPI CLEO_GetScriptDebugMode(const CLEO::CRunningScript* thread)
	{
		return reinterpret_cast<const CCustomScript*>(thread)->GetDebugMode();
	}

	void WINAPI CLEO_SetScriptDebugMode(CLEO::CRunningScript* thread, BOOL enabled)
	{
		reinterpret_cast<CCustomScript*>(thread)->SetDebugMode(enabled);
	}

	void WINAPI CLEO_GetScriptInfoStr(CLEO::CRunningScript* thread, bool currLineInfo, char* buf, DWORD bufSize)
	{
		if (thread == nullptr || buf == nullptr || bufSize < 2)
		{
			return; // invalid param
		}

		auto text = reinterpret_cast<CCustomScript*>(thread)->GetInfoStr(currLineInfo);

		if (text.length() >= bufSize)
			text.resize(bufSize - 1); // and terminator character

		std::memcpy(buf, text.c_str(), text.length() + 1); // with terminator
	}

	void WINAPI CLEO_GetScriptParamInfoStr(int idexOffset, char* buf, DWORD bufSize)
	{
		auto curr = idexOffset - 1 + GetInstance().OpcodeSystem.handledParamCount;
		auto name = GetInstance().OpcodeInfoDb.GetArgumentName(GetInstance().OpcodeSystem.lastOpcode, curr);

		curr++; // 1-based argument index display

		std::string msg;
		if (name != nullptr) msg = StringPrintf("#%d \"%s\"", curr, name);
		else msg = StringPrintf("#%d", curr);

		strncpy(buf, msg.c_str(), bufSize);
	}
}
