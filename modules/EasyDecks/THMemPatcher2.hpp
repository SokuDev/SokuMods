#pragma once
#include <Windows.h>
#include <memory>
#include <vector>

template<uintptr_t from, void (*to)(), size_t len = 5> struct jmp_t {
	static constexpr uintptr_t addr = from;
	static constexpr LPVOID func = to;
	static constexpr size_t length = len;
	static constexpr uintptr_t ret = from + len;
};

template<uintptr_t patch_addr, char... Bytes> struct patch_t {
	static constexpr uintptr_t addr = patch_addr;
	static constexpr char data[]{Bytes...};
	static constexpr size_t length{sizeof(data) / sizeof(char)};
};

class THMemPatcher2 {
private:
	struct Patch {
		uintptr_t addr = 0;
		std::vector<byte> new_data;
		std::vector<byte> orig_data;
		size_t len = 0;
		bool applied = 0;
		bool locked = 0;
	};

	using PatchList = std::vector<std::shared_ptr<Patch>>;
	PatchList patches;

	void UnlockAllPatches() {
		for (auto &&patch : patches)
			patch->locked = false;
	}

public:
	LPVOID VTBLHook(_In_ LPVOID dwEntry, _In_ LPVOID dwTarget, bool locked = false) {
		uintptr_t dwOrig = *reinterpret_cast<uintptr_t *>(dwEntry);
		AddPatch((uintptr_t)dwEntry, (const char *)&dwTarget, 4, locked);
		return (LPVOID)dwOrig;
	}

	template<uintptr_t addr, void (*func)(), size_t length> void AddDetourJump(jmp_t<addr, func, length> &jmp, bool locked = false) {
		AddDetourJump(jmp.addr, jmp.func, jmp.length, locked);
	}

	void AddDetourJump(uintptr_t at, LPVOID to, size_t len, bool locked = false) {
		BYTE *bSource = reinterpret_cast<BYTE *>(at);

		BYTE *bJump = (BYTE *)malloc(len);
		bJump[0] = 0xE9;
		*(DWORD *)(bJump + 1) = (DWORD)((BYTE *)to - bSource) - 5;
		for (size_t i = 5; i < len; i++)
			bJump[i] = 0x90;
		AddPatch((uintptr_t)bSource, (const char *)bJump, len);
		free(bJump);
	}

	template<uintptr_t addr, char... Args> void AddPatch(patch_t<addr, Args...> &patch, bool locked = false) {
		AddPatch(patch.addr, patch.data, patch.length);
	}

	void AddPatchNop(uintptr_t from, size_t len = 1, bool locked = false) {
		uintptr_t to = from + len;
		for (size_t i = from; i < to; ++i)
			AddPatch(i, "\x90", 1);
	}

	void AddPatch(uintptr_t addr, const char *data, size_t len, bool locked = false) {
		auto patch = std::make_shared<Patch>();
		patch->addr = addr;
		patch->len = len;
		patch->locked = locked;

		for (size_t i = 0; i < len; ++i) {
			patch->orig_data.push_back(static_cast<byte>(((char *)addr)[i]));
		}

		for (size_t i = 0; i < len; ++i) {
			patch->new_data.push_back(static_cast<byte>(((char *)data)[i]));
		}

		patches.emplace_back(std::move(patch));
	}

	void ClearPatches() {
		UnlockAllPatches();
		RestorePatches();
		patches.clear();
	}

	void RestorePatches() {
		DWORD oldProtect;
		for (auto &&patch : patches) {
			if (patch->locked || !patch->applied || !VirtualProtect((void *)patch->addr, patch->orig_data.size(), PAGE_EXECUTE_READWRITE, &oldProtect))
				continue;
			memcpy((void *)patch->addr, patch->orig_data.data(), patch->orig_data.size());
			patch->applied = false;
			VirtualProtect((void *)patch->addr, patch->orig_data.size(), oldProtect, &oldProtect);
		}
	}

	void ApplyPatches() {
		DWORD oldProtect;
		for (auto &&patch : patches) {
			if (patch->applied || !VirtualProtect((void *)patch->addr, patch->new_data.size(), PAGE_EXECUTE_READWRITE, &oldProtect))
				continue;
			memcpy((void *)patch->addr, patch->new_data.data(), patch->new_data.size());
			patch->applied = true;
			VirtualProtect((void *)patch->addr, patch->new_data.size(), oldProtect, &oldProtect);
		}
	}
};
