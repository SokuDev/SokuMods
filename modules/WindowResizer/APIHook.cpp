#include <windows.h>

template<typename D, typename S> inline D ShiftPointer(S source, ptrdiff_t diff) {
	return reinterpret_cast<D>(reinterpret_cast<DWORD_PTR>(source) + diff);
}

template<typename S, typename T> inline ptrdiff_t DiffPointer(S source, T target) {
	return reinterpret_cast<DWORD_PTR>(target) - reinterpret_cast<DWORD_PTR>(source);
}

bool SafeWriteMemory(LPVOID targetAddress, LPVOID targetData, DWORD targetSize, LPVOID originalData) {
	DWORD oldProtect;
	if (!::VirtualProtect(targetAddress, targetSize, PAGE_EXECUTE_READWRITE, &oldProtect))
		return false;
	DWORD ioSize;
	HANDLE currentProcess = GetCurrentProcess();
	if (originalData != NULL)
		ReadProcessMemory(currentProcess, targetAddress, originalData, targetSize, &ioSize);
	BOOL ret = WriteProcessMemory(currentProcess, targetAddress, targetData, targetSize, &ioSize);
	::VirtualProtect(targetAddress, targetSize, oldProtect, &oldProtect);
	return ret != FALSE;
}

PIMAGE_NT_HEADERS GetModuleNTHeaders(HMODULE targetModule) {
	PIMAGE_DOS_HEADER pdosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(targetModule);
	if (pdosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return NULL;
	PIMAGE_NT_HEADERS pntHeaders = ShiftPointer<PIMAGE_NT_HEADERS>(pdosHeader, pdosHeader->e_lfanew);
	if (pntHeaders->Signature != IMAGE_NT_SIGNATURE)
		return NULL;
	if (pntHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC)
		return NULL;
	return pntHeaders;
}

PIMAGE_DATA_DIRECTORY GetModuleDataDirectory(HMODULE targetModule, DWORD directoryIndex) {
	if (IMAGE_NUMBEROF_DIRECTORY_ENTRIES <= directoryIndex)
		return NULL;
	PIMAGE_NT_HEADERS32 pntHeaders = GetModuleNTHeaders(targetModule);
	if (pntHeaders == NULL)
		return NULL;
	return &pntHeaders->OptionalHeader.DataDirectory[directoryIndex];
}

LPVOID TamperImportAddressTable(HMODULE targetModule, PIMAGE_IMPORT_DESCRIPTOR ptargetDesc, LPCSTR targetProcName, FARPROC hookProc) {
	if (ptargetDesc->OriginalFirstThunk == 0 || ptargetDesc->FirstThunk == 0)
		return NULL;

	PIMAGE_THUNK_DATA pLookupTable = ShiftPointer<PIMAGE_THUNK_DATA>(targetModule, ptargetDesc->OriginalFirstThunk);
	PIMAGE_THUNK_DATA pAddressTable = ShiftPointer<PIMAGE_THUNK_DATA>(targetModule, ptargetDesc->FirstThunk);

	for (; pLookupTable->u1.Function != 0; ++pLookupTable, ++pAddressTable) {
		if (IMAGE_SNAP_BY_ORDINAL(pLookupTable->u1.Ordinal)) {
			if (reinterpret_cast<DWORD_PTR>(targetProcName) < 0x10000) {
				// Ord number comparison
				DWORD ordinalNum = IMAGE_ORDINAL(pLookupTable->u1.Ordinal);
				if (ordinalNum == reinterpret_cast<DWORD_PTR>(targetProcName)) {
					FARPROC orignalProc;
					if (SafeWriteMemory(&pAddressTable->u1.Function, reinterpret_cast<LPVOID>(&hookProc), sizeof(hookProc), reinterpret_cast<LPVOID *>(&orignalProc))) {
						return orignalProc;
					} else {
						return NULL;
					}
				}
			}
		} else {
			// Forward ref support is not implemented yet.
			if (reinterpret_cast<DWORD_PTR>(targetProcName) > 0x10000) {
				// Function name comparison
				PIMAGE_IMPORT_BY_NAME pfuncInfo = ShiftPointer<PIMAGE_IMPORT_BY_NAME>(targetModule, pLookupTable->u1.AddressOfData);
				if (::lstrcmpA(reinterpret_cast<LPCSTR>(pfuncInfo->Name), targetProcName) == 0) {
					FARPROC orignalProc;
					if (SafeWriteMemory(&pAddressTable->u1.Function, reinterpret_cast<LPVOID>(&hookProc), sizeof(hookProc), reinterpret_cast<LPVOID *>(&orignalProc))) {
						return orignalProc;
					} else {
						return NULL;
					}
				}
			}
		}
	}
	return NULL;
}

LPVOID HookAPICall(HMODULE targetModule, LPCSTR targetLibName, LPCSTR targetProcName, FARPROC hookProc) {
	PIMAGE_DATA_DIRECTORY pimportDataDirectory = GetModuleDataDirectory(targetModule, IMAGE_DIRECTORY_ENTRY_IMPORT);
	if (pimportDataDirectory == NULL)
		return NULL;
	if (pimportDataDirectory->Size == 0)
		return NULL;

	PIMAGE_IMPORT_DESCRIPTOR pimportDesc = ShiftPointer<PIMAGE_IMPORT_DESCRIPTOR>(targetModule, pimportDataDirectory->VirtualAddress);
	for (int i = 0; pimportDesc[i].Characteristics != 0; ++i) {
		if (pimportDesc[i].Name != 0) {
			LPCSTR libName = ShiftPointer<LPCSTR>(targetModule, pimportDesc[i].Name);
			if (::lstrcmpiA(targetLibName, libName) == 0) {
				return TamperImportAddressTable(targetModule, &pimportDesc[i], targetProcName, hookProc);
			}
		}
	}
	return NULL;
}

LPVOID HookAPIExport(HMODULE targetModule, LPCSTR targetProcName, FARPROC hookProc) {
	PIMAGE_DATA_DIRECTORY pexportDataDirectory = GetModuleDataDirectory(targetModule, IMAGE_DIRECTORY_ENTRY_EXPORT);
	if (pexportDataDirectory == NULL)
		return NULL;
	if (pexportDataDirectory->Size == 0)
		return NULL;

	PIMAGE_EXPORT_DIRECTORY pexportDirectory = ShiftPointer<PIMAGE_EXPORT_DIRECTORY>(targetModule, pexportDataDirectory->VirtualAddress);

	if (!pexportDirectory->AddressOfFunctions || !pexportDirectory->AddressOfNameOrdinals || !pexportDirectory->AddressOfNames)
		return NULL;
	LPDWORD pAddressTable = ShiftPointer<LPDWORD>(targetModule, pexportDirectory->AddressOfFunctions);
	LPWORD pOrdinalTable = ShiftPointer<LPWORD>(targetModule, pexportDirectory->AddressOfNameOrdinals);
	LPDWORD pNameTable = ShiftPointer<LPDWORD>(targetModule, pexportDirectory->AddressOfNames);

	for (DWORD i = 0; i < pexportDirectory->NumberOfFunctions; ++i) {
		if (pNameTable[i] != 0) {
			LPCSTR procName = ShiftPointer<LPCSTR>(targetModule, pNameTable[i]);
			if (::lstrcmpA(procName, targetProcName) == 0) {
				DWORD_PTR relativeHookProc = DiffPointer(targetModule, hookProc);
				DWORD_PTR relativeOrigProc;
				if (SafeWriteMemory(&pAddressTable[pOrdinalTable[i]], reinterpret_cast<LPVOID>(&relativeHookProc), sizeof(relativeHookProc),
							reinterpret_cast<LPVOID *>(&relativeOrigProc))) {
					return ShiftPointer<LPVOID>(targetModule, relativeOrigProc);
				} else {
					return NULL;
				}
			}
		}
	}

	return NULL;
}