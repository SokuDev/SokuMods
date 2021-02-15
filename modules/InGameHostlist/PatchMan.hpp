#pragma once
#include <Windows.h>
#include <iostream>
#include <list>
using namespace std;

namespace PatchMan {
using byte = unsigned char;

// From swrs.h
inline DWORD HookNear(DWORD addr, DWORD target) {
	DWORD oldProtect;
	if (!VirtualProtect((void *)addr, 5, PAGE_READWRITE, &oldProtect))
		return 0;

	DWORD old = (*(DWORD *)(addr + 1)) + (addr + 5);
	*((DWORD *)(addr + 1)) = target - (addr + 5);

	if (!VirtualProtect((void *)addr, 5, oldProtect, &oldProtect))
		return 0;

	return old;
}

class Patch {
private:
	void *address;
	byte *patch;
	byte *originalData;
	size_t size;

	byte *CopyData(const void *src, size_t size) {
		byte *p = new byte[size];
		memcpy(p, src, size);

		return p;
	}

public:
	Patch(DWORD addr, const string &patchStr) {
		address = (void *)addr;
		size = patchStr.size();
		patch = CopyData(patchStr.c_str(), size);
		originalData = CopyData(address, size);
	}

	Patch(DWORD addr, const char *patchStr, size_t sizeStr) {
		size = sizeStr;
		address = (void *)addr;
		patch = CopyData(patchStr, size);
		originalData = CopyData(address, size);
	}

	Patch(DWORD addr, byte patchByte, size_t count) {
		address = (void *)addr;
		size = count;
		patch = new byte[count];
		memset(patch, patchByte, size);
		originalData = CopyData(address, size);
	}

	~Patch() {
		delete patch;
		delete originalData;
	}

	bool Toggle(bool active) {
		DWORD oldProtect;
		if (VirtualProtect(address, size, PAGE_READWRITE, &oldProtect)) {
			memcpy(address, (active ? patch : originalData), size);
			return VirtualProtect(address, size, oldProtect, &oldProtect);
		} else
			return false;
	}

	bool Check() {
		return memcmp(address, patch, size) == 0;
	}
};

class MultiPatch {
private:
	list<Patch> patches;

public:
	MultiPatch &AddPatch(DWORD addr, const string &patch) {
		patches.emplace_back(addr, patch);
		return *this;
	}

	// Necessary if the string includes \0
	MultiPatch &AddPatch(DWORD addr, const char *patch, size_t size) {
		patches.emplace_back(addr, patch, size);
		return *this;
	}

	MultiPatch &AddNOPs(DWORD addr, size_t count) {
		patches.emplace_back(addr, (byte)0x90, count);
		return *this;
	}

	bool Check() {
		for (Patch &patch : patches)
			if (!patch.Check())
				return false;
		return true;
	}

	bool Toggle(bool active) {
		for (Patch &patch : patches)
			if (!patch.Toggle(active))
				return false;
		return true;
	}
};
} // namespace PatchMan