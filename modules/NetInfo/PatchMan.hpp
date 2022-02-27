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

	inline DWORD HookVTable(DWORD* addr, DWORD target) {
		DWORD oldProtect;
		if (!VirtualProtect(addr, 4, PAGE_READWRITE, &oldProtect))
			return 0;

		DWORD old = *addr;
		*addr = target;

		if (!VirtualProtect(addr, 4, oldProtect, &oldProtect))
			return 0;

		return old;
	}

	class Patch {
	private:
		void *address;
		byte *patch;
		byte *originalData;
		size_t size;
		bool state;

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
			state = false;
		}

		Patch(DWORD addr, const char *patchStr, size_t sizeStr) {
			size = sizeStr;
			address = (void *)addr;
			patch = CopyData(patchStr, size);
			originalData = CopyData(address, size);
			state = false;
		}

		Patch(DWORD addr, byte patchByte, size_t count) {
			address = (void *)addr;
			size = count;
			patch = new byte[count];
			memset(patch, patchByte, size);
			originalData = CopyData(address, size);
			state = false;
		}

		~Patch() {
			delete patch;
			delete originalData;
		}

		bool Toggle(bool new_state) {
			DWORD oldProtect;
			if (state != new_state) {
				if (VirtualProtect(address, size, PAGE_READWRITE, &oldProtect)) {
					memcpy(address, (new_state ? patch : originalData), size);
					state = new_state;
					return VirtualProtect(address, size, oldProtect, &oldProtect);
				}
				else
					return false;
			}
			else
				return true;
		}

		bool Check(bool deep_check) {
			if (deep_check)
				return memcmp(address, patch, size) == 0;
			else
				return state;
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

		bool Check(bool deep_check = false) {
			for (Patch &patch : patches)
				if (!patch.Check(deep_check))
					return false;
			return true;
		}

		bool Toggle(bool state) {
			for (Patch &patch : patches)
				if (!patch.Toggle(state))
					return false;
			return true;
		}
	};
} 