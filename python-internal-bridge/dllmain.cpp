#include <Windows.h>
#include <iostream>
#include <Python.h>
#include <string>

void* thiscall_wrapper(int class_address, int function_index, void* arg) {
	void*** this_ptr{ reinterpret_cast<void***>(class_address) };
	void* function{ (*this_ptr)[function_index] };

	if (arg) _asm push arg; //msvc gets fucky here so doing it with asm is easier
	__asm {
		mov ecx, this_ptr
		call function
	}
}

unsigned long __stdcall main_thread(HMODULE hModule) {
	AllocConsole();
	FILE* file_pointer{};
	freopen_s(&file_pointer, "CONOUT$", "w", stdout);
	freopen_s(&file_pointer, "CONOUT$", "w", stderr);
	freopen_s(&file_pointer, "CONIN$", "r", stdin);

	Py_Initialize();
	std::string init_string{};
	init_string += "import ctypes\n";
	init_string += "ctypes.cdll.client.w = ctypes.CFUNCTYPE(ctypes.c_void_p)(" + std::to_string(reinterpret_cast<unsigned long>(&thiscall_wrapper)) + ")" +'\n';
	PyRun_SimpleString(init_string.c_str());

	while (true) {
		std::cout << "> ";
		std::string input{};
		std::getline(std::cin, input);
		if (input == "pib-quit") break;
		PyRun_SimpleString(input.c_str());
	}

	Py_FinalizeEx();

	if (file_pointer) fclose(file_pointer);
	FreeConsole();
	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}

bool __stdcall DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
		HANDLE thread_handle{ CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(main_thread), hModule, 0, NULL) };
		if (thread_handle) CloseHandle(thread_handle);
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

