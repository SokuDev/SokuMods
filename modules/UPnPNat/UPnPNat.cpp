#include <winsock2.h>
#include <cstdio>
#include <iostream>
#include <iphlpapi.h>
#include <natupnp.h>
#include <shlwapi.h>
#include <vector>

const BYTE TARGET_HASH[16] = {0xdf, 0x35, 0xd1, 0xfb, 0xc7, 0xb5, 0x83, 0x31, 0x7a, 0xda, 0xbe, 0x8c, 0xd9, 0xf5, 0x3b, 0x2e};
IUPnPNAT *natDevice = 0;
IStaticPortMappingCollection *collection = 0;
std::vector<WCHAR *> ipAddresses;
std::vector<IStaticPortMapping *> mappings;
std::string modulePath;
HANDLE mutex;
struct {
	int externalPort;
	int internalPort;
} iniConfig;

bool LoadAddresses() {
	ULONG bufferSize;
	ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;
	if (GetAdaptersAddresses(AF_INET, flags, 0, 0, &bufferSize) != ERROR_BUFFER_OVERFLOW) {
		return false;
	}

	PIP_ADAPTER_ADDRESSES pList = (PIP_ADAPTER_ADDRESSES)malloc(bufferSize);
	if (GetAdaptersAddresses(AF_INET, flags, 0, pList, &bufferSize) != ERROR_SUCCESS) {
		free(pList);
		return false;
	}

	for (PIP_ADAPTER_ADDRESSES adapter = pList; adapter; adapter = adapter->Next) {
		if (adapter->OperStatus != IfOperStatusUp)
			continue;
		for (PIP_ADAPTER_UNICAST_ADDRESS addr = adapter->FirstUnicastAddress; addr; addr = addr->Next) {
			if (addr->Address.lpSockaddr->sa_family != AF_INET) {
				continue;
			}
			SOCKADDR_IN *addr_in = (SOCKADDR_IN *)addr->Address.lpSockaddr;
			if (addr_in->sin_addr.S_un.S_un_b.s_b1 == 127) { // ignore localhost addresses
				continue;
			}
			WCHAR *buffer = new WCHAR[16];
			wsprintfW(buffer, L"%d.%d.%d.%d", addr_in->sin_addr.S_un.S_un_b.s_b1, addr_in->sin_addr.S_un.S_un_b.s_b2, addr_in->sin_addr.S_un.S_un_b.s_b3,
				addr_in->sin_addr.S_un.S_un_b.s_b4);
			ipAddresses.push_back(buffer);
		}
	}

	return ipAddresses.size() > 0;
}

void UnloadAddresses() {
	for (auto &name : ipAddresses) {
		delete[] name;
	}
	ipAddresses.clear();
}

bool LoadUPNP() {
	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED))) {
		return false;
	}
	if (FAILED(CoCreateInstance(CLSID_UPnPNAT, NULL, CLSCTX_ALL, IID_IUPnPNAT, (void **)&natDevice))) {
		return false;
	}
	if (natDevice->get_StaticPortMappingCollection(&collection) != S_OK) {
		return false;
	}
	if (collection == nullptr) {
		return false;
	}

	BSTR udpStr = SysAllocString(L"UDP");
	for (auto &ipAddr : ipAddresses) {
		WCHAR ruleName[23];
		wcscpy(ruleName, L"Soku: ");
		wcscpy(&ruleName[6], ipAddr);
		BSTR ruleStr = SysAllocString(ruleName);
		BSTR ipStr = SysAllocString(ipAddr);
		IStaticPortMapping *mapping = 0;
		if (collection->Add(iniConfig.externalPort, udpStr, iniConfig.internalPort, ipStr, true, ruleStr, &mapping) == S_OK) {
			mappings.push_back(mapping);
		}
		SysFreeString(ipStr);
		SysFreeString(ruleStr);
	}
	SysFreeString(udpStr);

	return mappings.size() > 0;
}

void UnloadUPNP() {
	for (auto &mapping : mappings)
		mapping->Release();
	mappings.clear();
	if (collection)
		collection->Release();
	if (natDevice)
		natDevice->Release();
}

void LoadSettings() {
	std::string configPath(modulePath + "\\UPnPNat.ini");
	if (!PathFileExists(configPath.c_str())) {
		WritePrivateProfileString("Options", "externalPort", "10800", configPath.c_str());
		WritePrivateProfileString("Options", "internalPort", "10800", configPath.c_str());
	}

	iniConfig.externalPort = GetPrivateProfileInt("Options", "externalPort", 10800, configPath.c_str());
	iniConfig.internalPort = GetPrivateProfileInt("Options", "internalPort", 10800, configPath.c_str());
}

DWORD WINAPI Load(LPVOID lpParam) {
	UNREFERENCED_PARAMETER(lpParam);

	LoadSettings();

	WaitForSingleObject(mutex, INFINITE);
	LoadAddresses();
	ReleaseMutex(mutex);

	WaitForSingleObject(mutex, INFINITE);
	if (ipAddresses.size() > 0) {
		LoadUPNP();
	}
	ReleaseMutex(mutex);

	return 0;
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return ::memcmp(TARGET_HASH, hash, sizeof TARGET_HASH) == 0;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	char tmpPath[MAX_PATH];
	GetModuleFileNameA(hMyModule, tmpPath, MAX_PATH);
	PathRemoveFileSpecA(tmpPath);
	modulePath = tmpPath;

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Load, NULL, 0, NULL);
	return TRUE;
}

extern "C" __declspec(dllexport) void AtExit() {
	WaitForSingleObject(mutex, INFINITE);
	if (mappings.size() > 0) {
		UnloadUPNP();
	}
	ReleaseMutex(mutex);

	WaitForSingleObject(mutex, INFINITE);
	UnloadAddresses();
	ReleaseMutex(mutex);
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		mutex = CreateMutex(NULL, FALSE, NULL);
		break;
	case DLL_PROCESS_DETACH:
		CloseHandle(mutex);
		break;
	default:;
	}
	return TRUE;
}
