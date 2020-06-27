#pragma once
#include <windows.h>
#include <memory>
#include <vector>

template<uintptr_t from, void (*to)(), size_t len> struct jmp_t {
	static constexpr uintptr_t addr = from;
	static constexpr LPVOID func = to;
	static constexpr size_t length = len;
	static constexpr uintptr_t ret = from + len;
};

class THMemPatcher {
private:
	struct Patch {
		uintptr_t addr = 0;
		std::vector<char> orig_data;
	};

	std::vector<std::shared_ptr<Patch>> patches;

	void AddPatch(uintptr_t addr, size_t len) {
		auto patch = std::make_shared<Patch>();
		patch->addr = addr;

		for (size_t i = 0; i < len; ++i) {
			patch->orig_data.push_back(((char *)addr)[i]);
		}

		patches.emplace_back(std::move(patch));
	}

public:
	bool PatchMemory(uintptr_t addr, const char *data, size_t len) {
		DWORD oldProtect;
		if (!VirtualProtect((void *)addr, len, PAGE_READWRITE, &oldProtect))
			return false;

		AddPatch(addr, len);
		memcpy((void *)addr, data, len);

		return VirtualProtect((void *)addr, len, oldProtect, &oldProtect);
	}

	void Nop(uintptr_t from, size_t len = 1) {
		uintptr_t to = from + len;
		for (size_t i = from; i < to; ++i) {
			PatchMemory(i, "\x90", 1);
		}
	}

	template<uintptr_t addr, void (*func)(), size_t length> void ApplyDetourJump(jmp_t<addr, func, length> &jmp) {
		DetourJump(jmp.addr, jmp.func, jmp.length);
	}

	void DetourJump(uintptr_t at, LPVOID to, size_t len) {
		BYTE *bSource = reinterpret_cast<BYTE *>(at);

		AddPatch((uintptr_t)bSource, len);

		DWORD dwOldProtection = NULL;
		BYTE *bJump = (BYTE *)malloc(len + 5);
		VirtualProtect(bSource, len, PAGE_EXECUTE_READWRITE, &dwOldProtection);
		memcpy(bJump, bSource, len);

		bJump[len] = 0xE9;
		*(DWORD *)(bJump + len) = (DWORD)((bSource + len) - (bJump + len)) - 5;

		bSource[0] = 0xE9;
		*(DWORD *)(bSource + 1) = (DWORD)((BYTE *)to - bSource) - 5;

		for (size_t i = 5; i < len; i++)
			bSource[i] = 0x90;

		VirtualProtect(bSource, len, dwOldProtection, &dwOldProtection);
	}

	void Restore() {
		for (auto &&patch : patches) {
			DWORD oldProtect;
			if (!VirtualProtect((void *)patch->addr, patch->orig_data.size(), PAGE_READWRITE, &oldProtect))
				continue;

			memcpy((void *)patch->addr, patch->orig_data.data(), patch->orig_data.size());

			VirtualProtect((void *)patch->addr, patch->orig_data.size(), oldProtect, &oldProtect);
		}

		patches.clear();
	}
};
