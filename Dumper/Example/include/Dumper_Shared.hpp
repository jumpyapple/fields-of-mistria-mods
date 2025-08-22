#pragma once
#include <YYToolkit/YYTK_Shared.hpp>
#include <filesystem>

#ifdef DUMPER_EXPORTS
#define DUMPER_API extern "C" __declspec(dllexport)
#else
#define DUMPER_API extern "C" __declspec(dllimport)
#include <windows.h>
#endif

/**
* Create a hook that will dump the variables automatically after the original function ends.
*
* Need a global static variable g_SHOULD_DUMP.
*
*/
#ifdef DUMPER_EXPORTS
#define DEFINE_DUMPING_HOOK_FUNCTION(FuncName, PluginName, FolderPrefix, HookIdentifier, CounterVarName) \
YYTK::RValue& FuncName(IN YYTK::CInstance* Self, IN YYTK::CInstance* Other, OUT YYTK::RValue& Result, IN int ArgumentCount, IN YYTK::RValue** Arguments) { \
	g_ModuleInterface->Print(YYTK::CM_LIGHTGREEN, "[%s %s] - '%s' is called!", PluginName, VERSION, HookIdentifier); \
	const YYTK::PFUNC_YYGMLScript original = reinterpret_cast<YYTK::PFUNC_YYGMLScript>(Aurie::MmGetHookTrampoline(Aurie::g_ArSelfModule, HookIdentifier)); \
	original(Self, Other, Result, ArgumentCount, Arguments); \
	if (g_SHOULD_DUMP) { \
		Dumper::DumpHookVariables(g_ModuleInterface, FolderPrefix, CounterVarName, Self, Other, Result, ArgumentCount, Arguments); \
		CounterVarName++; \
	} \
	return Result; \
}
#else
#define DEFINE_DUMPING_HOOK_FUNCTION(FuncName, PluginName, FolderPrefix, HookIdentifier, CounterVarName) \
YYTK::RValue& FuncName(IN YYTK::CInstance* Self, IN YYTK::CInstance* Other, OUT YYTK::RValue& Result, IN int ArgumentCount, IN YYTK::RValue** Arguments) { \
	g_ModuleInterface->Print(YYTK::CM_LIGHTGREEN, "[%s %s] - '%s' is called!", PluginName, VERSION, HookIdentifier); \
	const YYTK::PFUNC_YYGMLScript original = reinterpret_cast<YYTK::PFUNC_YYGMLScript>(Aurie::MmGetHookTrampoline(Aurie::g_ArSelfModule, HookIdentifier)); \
	original(Self, Other, Result, ArgumentCount, Arguments); \
	if (g_SHOULD_DUMP) { \
		Dumper::CallDumpHookVariables(g_ModuleInterface, FolderPrefix, CounterVarName, Self, Other, Result, ArgumentCount, Arguments); \
		CounterVarName++; \
	} \
	return Result; \
}
#endif

namespace Dumper {

	namespace fs = std::filesystem;

	/**
	* @brief Dump the details of the Instance into target_directory
	*
	* The details of the given instenace will be put in index.html file while
	* the details for other related instances are put in a HTML file with pointer (in uint64)
	* as its name.
	*
	* @param Instance The instance to start dumping from.
	* @param target_directory The directory to store all the HTML files.
	*/
	DUMPER_API void DumpRValue(YYTK::YYTKInterface* g_ModuleInterface, YYTK::RValue& Value, fs::path target_directory, std::string index_filename);

	DUMPER_API void DumpRValueWithDefaultIndexFilename(YYTK::YYTKInterface* g_ModuleInterface, YYTK::RValue& Value, fs::path target_directory);

	DUMPER_API void DumpCInstance(YYTK::YYTKInterface* g_ModuleInterface, YYTK::CInstance* Instance, fs::path target_directory, std::string index_filename);

	DUMPER_API void DumpCInstanceWithDefaultIndexFilename(YYTK::YYTKInterface* g_ModuleInterface, YYTK::CInstance* Instance, fs::path target_directory);

	/**
	* Dump all variables from the hook.
	*
	* The Prefix is the prefix for the folder name. This function will create at least
	* 3 folders (one for Self, one for Other and one for Result). A folder is
	* create for each of the Arguments.
	*
	* The Count is there so that multiple copy of the index file can exist. You should increase
	* the Count each time the hook is executed.
	*
	* Currently, we assume that the address will point to the same thing across the calls.
	*/
	DUMPER_API void DumpHookVariables(YYTK::YYTKInterface* g_ModuleInterface, std::string Prefix, uint64_t Count, YYTK::CInstance* Self, YYTK::CInstance* Other, YYTK::RValue Result, int ArgumentCount, YYTK::RValue** Arguments);

#ifndef DUMPER_EXPORTS
	static HMODULE DumperModule = nullptr;

	typedef void (*DumpRValueT)(YYTK::YYTKInterface*, YYTK::RValue&, fs::path, std::string);
	bool CallDumpRValue(YYTK::YYTKInterface* g_ModuleInterface, YYTK::RValue& Value, fs::path target_directory, std::string index_filename) {
		if (DumperModule == nullptr) {
			// We dont know the address yet.
			DumperModule = GetModuleHandleA("Dumper.dll");

			if (DumperModule == nullptr) {
				return false;
			}
		}

		// Get the function pointer?
		DumpRValueT ptr = (DumpRValueT)GetProcAddress(DumperModule, "DumpRValue");

		if (ptr != nullptr) {
			ptr(g_ModuleInterface, Value, target_directory, index_filename);
		}
		else {
			return false;
		}
		return true;
	}

	typedef void (*DumpRValueWithDefaultIndexFilenameT)(YYTK::YYTKInterface*, YYTK::RValue&, fs::path);
	bool CallDumpRValueWithDefaultIndexFilename(YYTK::YYTKInterface* g_ModuleInterface, YYTK::RValue& Value, fs::path target_directory) {
		if (DumperModule == nullptr) {
			// We dont know the address yet.
			DumperModule = GetModuleHandleA("Dumper.dll");

			if (DumperModule == nullptr) {
				return false;
			}
		}

		// Get the function pointer?
		DumpRValueWithDefaultIndexFilenameT ptr = (DumpRValueWithDefaultIndexFilenameT)GetProcAddress(DumperModule, "DumpRValueWithDefaultIndexFilename");

		if (ptr != nullptr) {
			ptr(g_ModuleInterface, Value, target_directory);
		}
		else {
			return false;
		}
		return true;
	}

	typedef void (*DumpCInstanceT)(YYTK::YYTKInterface*, YYTK::CInstance*, fs::path, std::string);
	bool CallDumpCInstance(YYTK::YYTKInterface* g_ModuleInterface, YYTK::CInstance* Instance, fs::path target_directory, std::string index_filename) {
		if (DumperModule == nullptr) {
			// We dont know the address yet.
			DumperModule = GetModuleHandleA("Dumper.dll");

			if (DumperModule == nullptr) {
				return false;
			}
		}

		// Get the function pointer?
		DumpCInstanceT ptr = (DumpCInstanceT)GetProcAddress(DumperModule, "DumpCInstance");

		if (ptr != nullptr) {
			ptr(g_ModuleInterface, Instance, target_directory, index_filename);
		}
		else {
			return false;
		}
		return true;
	}

	typedef void (*DumpCInstanceWithDefaultIndexFilenameT)(YYTK::YYTKInterface*, YYTK::CInstance*, fs::path);
	bool CallDumpCInstanceWithDefaultIndexFilename(YYTK::YYTKInterface* g_ModuleInterface, YYTK::CInstance* Instance, fs::path target_directory) {
		if (DumperModule == nullptr) {
			// We dont know the address yet.
			DumperModule = GetModuleHandleA("Dumper.dll");

			if (DumperModule == nullptr) {
				return false;
			}
		}

		// Get the function pointer?
		DumpCInstanceWithDefaultIndexFilenameT ptr = (DumpCInstanceWithDefaultIndexFilenameT)GetProcAddress(DumperModule, "DumpCInstanceWithDefaultIndexFilename");

		if (ptr != nullptr) {
			ptr(g_ModuleInterface, Instance, target_directory);
		}
		else {
			return false;
		}
		return true;
	}

	typedef void (*DumpHookVariablesT)(YYTK::YYTKInterface*, std::string, uint64_t, YYTK::CInstance*, YYTK::CInstance*, YYTK::RValue, int, YYTK::RValue**);
	bool CallDumpHookVariables(YYTK::YYTKInterface* g_ModuleInterface, std::string Prefix, uint64_t Count, YYTK::CInstance* Self, YYTK::CInstance* Other, YYTK::RValue Result, int ArgumentCount, YYTK::RValue** Arguments) {
		if (DumperModule == nullptr) {
			// We dont know the address yet.
			DumperModule = GetModuleHandleA("Dumper.dll");

			if (DumperModule == nullptr) {
				return false;
			}
		}

		// Get the function pointer?
		DumpHookVariablesT ptr = (DumpHookVariablesT)GetProcAddress(DumperModule, "DumpHookVariables");

		if (ptr != nullptr) {
			ptr(g_ModuleInterface, Prefix, Count, Self, Other, Result, ArgumentCount, Arguments);
		}
		else {
			return false;
		}
		return true;
	}
#endif
}
