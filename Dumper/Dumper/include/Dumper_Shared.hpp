#pragma once
#include <YYToolkit/YYTK_Shared.hpp>
#include <filesystem>

#ifdef DUMPER_EXPORTS
#define DUMPER_API extern "C" __declspec(dllexport)
#else
#define DUMPER_API extern "C" __declspec(dllimport)
#include <windows.h>
#endif

namespace Dumper {

	namespace fs = std::filesystem;

	DUMPER_API void EnableDumper();
	DUMPER_API void DisableDumper();

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
	DUMPER_API void DumpRValue(YYTK::YYTKInterface* g_ModuleInterface, YYTK::RValue& Instance, fs::path target_directory, std::string index_filename);

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

	typedef void (*EnableDumperT)();
	typedef void (*DisableDumperT)();
	typedef void (*DumpHookVariablesT)(YYTK::YYTKInterface* g_ModuleInterface, std::string Prefix, uint64_t Count, YYTK::CInstance* Self, YYTK::CInstance* Other, YYTK::RValue Result, int ArgumentCount, YYTK::RValue** Arguments);

	bool CallEnableDumper() {
		if (DumperModule == nullptr) {
			DumperModule = GetModuleHandleA("Dumper.dll");

			if (DumperModule == nullptr) {
				return false;
			}
		}

		EnableDumperT ptr = (EnableDumperT)GetProcAddress(DumperModule, "EnableDumper");
		if (ptr != nullptr) {
			ptr();
		}
		else {
			return false;
		}
		return true;
	}

	bool CallDisableDumper() {
		if (DumperModule == nullptr) {
			DumperModule = GetModuleHandleA("Dumper.dll");

			if (DumperModule == nullptr) {
				return false;
			}
		}

		EnableDumperT ptr = (EnableDumperT)GetProcAddress(DumperModule, "DisableDumper");
		if (ptr != nullptr) {
			ptr();
		}
		else {
			return false;
		}
		return true;
	}

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