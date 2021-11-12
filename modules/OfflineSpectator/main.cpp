//
// Created by Gegel85 on 04/12/2020
//

#include <Windows.h>
#include <Shlwapi.h>
#include <algorithm>
#include <dinput.h>
#include <SokuLib.hpp>
#include <iostream>
#include <mutex>
#include <fstream>
#include <zlib.h>
#include <process.h>

//Hooks
static int (__stdcall *realRecvfrom)(SOCKET s, char *buf, int len, int flags, sockaddr *from, int *fromlen);
static int (__stdcall *realSendto)(SOCKET s, char *buf, int len, int flags, sockaddr *to, int tolen);
static int (SokuLib::Battle::*s_origCBattle_Process)();
static int (SokuLib::Loading::*s_origCLoading_Process)();
static void (SokuLib::KeymapManager::*s_origKeymapManager_SetInputs)();

//Game state
static std::array<std::vector<std::pair<SokuLib::BattleKeys, SokuLib::BattleKeys>>, 64> replays;
static int gameId = 0;
static bool finished = true;
static bool loaded = false;
static std::mutex mutex;
static SOCKET sock = INVALID_SOCKET;
static unsigned short hostPort = 10800;

static std::ofstream stream{"speclog.txt"};

class ZUtils {
public:
	static constexpr long int CHUNK = {16384};

	static int compress(byte *inBuffer, size_t size, std::vector<byte> &outBuffer, int level)
	{
		int ret, flush;
		unsigned have;
		z_stream strm;
		unsigned char out[CHUNK];

		outBuffer.clear();
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		ret = deflateInit(&strm, level);
		if (ret != Z_OK)
			return ret;

		strm.avail_in = size;
		strm.next_in = inBuffer;
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = deflate(&strm, Z_FINISH);    /* anyone error value */
			assert(ret != Z_STREAM_ERROR);
			have = CHUNK - strm.avail_out;
			outBuffer.insert(outBuffer.end(), out, out + have);
		} while (strm.avail_out == 0);
		assert(strm.avail_in == 0);
		assert(ret == Z_STREAM_END);
		deflateEnd(&strm);
		return Z_OK;
	}

	static int decompress(byte *inBuffer, size_t size, std::vector<byte> &outBuffer)
	{
		int ret;
		unsigned have;
		z_stream strm;
		unsigned char out[CHUNK];

		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		strm.avail_in = 0;
		strm.next_in = Z_NULL;
		ret = inflateInit(&strm);
		if (ret != Z_OK)
			return ret;

		strm.avail_in = size;
		strm.next_in = inBuffer;

		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);
			switch (ret) {
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				inflateEnd(&strm);
				return ret;
			}
			have = CHUNK - strm.avail_out;
			outBuffer.insert(outBuffer.end(), out, out + have);
		} while (strm.avail_out == 0);
		assert (ret == Z_STREAM_END);

		inflateEnd(&strm);
		return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
	}

	static void zerror(int ret)
	{
		std::cerr << "zpipe: ";
		switch (ret) {
		case Z_ERRNO:
			if (ferror(stdin)) {
				std::cerr << "Error reading from stdin." << std::endl;
				//stream << "Error reading from stdin." << std::endl;
			} else if (ferror(stdout)) {
				std::cerr << "Error writing ro stdout." << std::endl;
				//stream << "Error writing ro stdout." << std::endl;
			}
			break;
		case Z_STREAM_ERROR:
			std::cerr << "Invalid compression level." << std::endl;
			//stream << "Invalid compression level." << std::endl;
			break;
		case Z_DATA_ERROR:
			std::cerr << "Empty data, invalid or incomplete." << std::endl;
			//stream << "Empty data, invalid or incomplete." << std::endl;
			break;
		case Z_MEM_ERROR:
			std::cerr << "No memory." << std::endl;
			//stream << "No memory." << std::endl;
			break;
		case Z_VERSION_ERROR:
			std::cerr << "zlib version is incompatible." << std::endl;
			//stream << "zlib version is incompatible." << std::endl;
		}
	}
};

int __stdcall fakeRecvfrom(SOCKET s, char *buf, int len, int flags, sockaddr *from, int *fromlen)
{
	int ret = realRecvfrom(s, buf, len, flags, from, fromlen);

	if (ret > 0) {
		std::vector<byte> result;
		auto pack = reinterpret_cast<SokuLib::Packet *>(buf);

		printf("RecvFrom -> ");
		SokuLib::displayPacketContent(std::cout, *pack);
		stream << "RecvFrom -> ";
		SokuLib::displayPacketContent(stream, *pack);

		if (pack->type == SokuLib::HOST_GAME && pack->game.event.type == SokuLib::GAME_REPLAY) {
			auto ret = ZUtils::decompress(pack->game.event.replay.compressedData, pack->game.event.replay.replaySize, result);

			if (ret != Z_OK)
				ZUtils::zerror(ret);
			else {
				struct Test {
					unsigned frameId;
					unsigned maxFrameId;
					unsigned char gameId;
					unsigned char length;
					unsigned short data[1];
				};
				auto a = reinterpret_cast<Test *>(result.data());

				std::cout << std::hex;
				stream << std::hex;
				for (byte b : result) {
					std::cout << " 0x" << +b;
					stream << " 0x" << +b;
				}
				std::cout << std::dec;
				stream << std::dec;

				std::cout << " frameId: " << a->frameId;
				stream << " frameId: " << a->frameId;
				std::cout << " maxFrameId: " << a->maxFrameId;
				stream << " maxFrameId: " << a->maxFrameId;
				std::cout << " gameId: " << +a->gameId;
				stream << " gameId: " << +a->gameId;
				std::cout << " length: " << +a->length;
				stream << " length: " << +a->length;
				std::cout << " inputs:";
				stream << " inputs:";

				for (int i = 0; i < a->length; i += 2) {
					std::cout << " 0x" << a->data[i] << "|0x" << a->data[i + 1];
					stream << " 0x" << a->data[i] << "|0x" << a->data[i + 1];
				}
			}
		}
		std::cout << std::endl;
		stream << std::endl;
	}
	return ret;
}

int __stdcall fakeSendto(SOCKET s, char *buf, int len, int flags, sockaddr *to, int tolen)
{
	std::vector<byte> result;
	auto pack = reinterpret_cast<SokuLib::Packet *>(buf);

	printf("SendTo -> ");
	SokuLib::displayPacketContent(std::cout, *reinterpret_cast<SokuLib::Packet *>(buf));
	stream << "SendTo -> ";
	SokuLib::displayPacketContent(stream, *reinterpret_cast<SokuLib::Packet *>(buf));
	if (pack->type == SokuLib::HOST_GAME && pack->game.event.type == SokuLib::GAME_REPLAY) {
		auto ret = ZUtils::decompress(pack->game.event.replay.compressedData, pack->game.event.replay.replaySize, result);

		if (ret != Z_OK)
			ZUtils::zerror(ret);
		else {
			struct Test {
				unsigned frameId;
				unsigned maxFrameId;
				unsigned char gameId;
				unsigned char length;
				unsigned short data[1];
			};
			auto a = reinterpret_cast<Test *>(result.data());

			std::cout << std::hex;
			stream << std::hex;
			for (byte b : result) {
				std::cout << " 0x" << +b;
				stream << " 0x" << +b;
			}
			std::cout << std::dec;
			stream << std::dec;

			std::cout << " frameId: " << a->frameId;
			stream << " frameId: " << a->frameId;
			std::cout << " maxFrameId: " << a->maxFrameId;
			stream << " maxFrameId: " << a->maxFrameId;
			std::cout << " gameId: " << +a->gameId;
			stream << " gameId: " << +a->gameId;
			std::cout << " length: " << +a->length;
			stream << " length: " << +a->length;
			std::cout << " inputs:";
			stream << " inputs:";

			for (int i = 0; i < a->length; i += 2) {
				std::cout << " 0x" << a->data[i] << "|0x" << a->data[i + 1];
				stream << " 0x" << a->data[i] << "|0x" << a->data[i + 1];
			}
		}
	}
	std::cout << std::endl;
	stream << std::endl;
	return realSendto(s, buf, len, flags, to, tolen);
}

std::string getLastSocketError(int err = WSAGetLastError()) {
	char *s = nullptr;

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&s,
		0,
		nullptr
	);

	auto result = "WSAGetLastError " + std::to_string(err) + ": " + s;

	LocalFree(s);
	return result;
}

void handleInitRequest(SokuLib::Packet &packet, sockaddr_in &addr)
{
	if (memcmp(
		packet.initRequest.gameId,
		SokuLib::SWRUnlinked ? SokuLib::Soku110acNoSWRAllChars : SokuLib::Soku110acRollSWRAllChars,
		sizeof(packet.initRequest.gameId)
	) != 0) {
		puts("Invalid game version");
		//Invalid game version
		return;
	}
	if (packet.initRequest.reqType != SokuLib::SPECTATE_REQU) {
		packet.type = SokuLib::INIT_ERROR;
		packet.initError.reason = SokuLib::ERROR_GAME_STATE_INVALID;
		SokuLib::DLL::ws2_32.sendto(sock, reinterpret_cast<char *>(&packet), sizeof(packet.initError), 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
		return;
	}
	packet.type = SokuLib::INIT_SUCCESS;
	packet.initSuccess.unknown1[0] = 0x0;
	packet.initSuccess.unknown1[1] = 0x0;
	packet.initSuccess.unknown1[2] = 0x0;
	packet.initSuccess.unknown1[3] = 0x40;
	packet.initSuccess.unknown1[4] = 0x11;
	packet.initSuccess.unknown1[5] = 0x0;
	packet.initSuccess.unknown1[6] = 0x0;
	packet.initSuccess.unknown1[7] = 0x0;
	packet.initSuccess.dataSize = 68;
	packet.initSuccess.unknown2[0] = 0x0;
	packet.initSuccess.unknown2[1] = 0xFF;
	packet.initSuccess.unknown2[2] = 0x4;
	strncpy(packet.initSuccess.hostProfileName, SokuLib::profile1.name, 32);
	strncpy(packet.initSuccess.clientProfileName, SokuLib::profile2.name, 32);
	packet.initSuccess.swrDisabled = SokuLib::SWRUnlinked;
	SokuLib::DLL::ws2_32.sendto(sock, reinterpret_cast<char *>(&packet), sizeof(packet.initSuccess), 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
}

void sendMatchInfo(SokuLib::Packet &packet, sockaddr_in &addr)
{
	packet.game.event.type = SokuLib::GAME_MATCH;
	packet.game.event.match.host.character                    = static_cast<SokuLib::CharacterPacked>(SokuLib::leftPlayerInfo.character);
	packet.game.event.match.host.deckId                       = SokuLib::leftPlayerInfo.deck;
	packet.game.event.match.host.deckSize                     = SokuLib::leftPlayerInfo.effectiveDeck.size;
	packet.game.event.match.host.skinId                       = SokuLib::leftPlayerInfo.palette;
	packet.game.event.match.host.disabledSimultaneousButton() = false;
	for (int i = 0; i < SokuLib::leftPlayerInfo.effectiveDeck.size; i++)
		packet.game.event.match.host.cards[i] = SokuLib::leftPlayerInfo.effectiveDeck[i];

	packet.game.event.match.client().character                    = static_cast<SokuLib::CharacterPacked>(SokuLib::rightPlayerInfo.character);
	packet.game.event.match.client().deckId                       = SokuLib::rightPlayerInfo.deck;
	packet.game.event.match.client().deckSize                     = SokuLib::rightPlayerInfo.effectiveDeck.size;
	packet.game.event.match.client().skinId                       = SokuLib::rightPlayerInfo.palette;
	packet.game.event.match.client().disabledSimultaneousButton() = false;
	for (int i = 0; i < SokuLib::rightPlayerInfo.effectiveDeck.size; i++)
		packet.game.event.match.client().cards[i] = SokuLib::rightPlayerInfo.effectiveDeck[i];

	packet.game.event.match.stageId()    = SokuLib::gameParams.stageId;
	packet.game.event.match.musicId()    = SokuLib::gameParams.musicId;
	packet.game.event.match.randomSeed() = SokuLib::gameParams.randomSeed;
	packet.game.event.match.matchId()    = gameId;

	auto size = (sizeof(SokuLib::PlayerMatchData) - 2) * 2 + 7 + 4 + 2 * (SokuLib::leftPlayerInfo.effectiveDeck.size + SokuLib::rightPlayerInfo.effectiveDeck.size);

	packet.type = SokuLib::HOST_GAME;
	SokuLib::DLL::ws2_32.sendto(sock, reinterpret_cast<char *>(&packet), size, 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
}

std::pair<byte *, unsigned> getReplayData(unsigned frameId)
{
	int length = min(60, replays[gameId].size() * 2 - frameId);
	auto size = sizeof(SokuLib::ReplayData) + length * 2;
	byte *data = new byte[size];
	auto pack = reinterpret_cast<SokuLib::ReplayData *>(data);

	pack->matchId = gameId;
	pack->frameId = frameId + length;
	pack->endFrameId = finished ? replays[gameId].size() : 0;
	pack->length = length;
	for (int i = 0; i < length; i += 2) {
		if (i + frameId < replays[gameId].size()) {
			pack->replayInputs[i].battle = replays[gameId][i + frameId].first;
			pack->replayInputs[i + 1].battle = replays[gameId][i + frameId].second;
		}
	}
	return {data, size};
}

void handleClientGame(SokuLib::Packet &packet, sockaddr_in &addr)
{
	if (packet.game.event.type != SokuLib::GAME_REPLAY_REQUEST)
		return;

	if (packet.game.event.replayRequest.matchId != gameId)
		return sendMatchInfo(packet, addr);

	mutex.lock();
	std::vector<byte> result;
	std::pair<byte *, unsigned> data = getReplayData(packet.game.event.replayRequest.frameId);
	mutex.unlock();

	ZUtils::compress(data.first, data.second, result, 1);
	delete[] data.first;
	packet.type = SokuLib::HOST_GAME;
	packet.game.event.type = SokuLib::GAME_REPLAY;
	packet.game.event.replay.replaySize = result.size();
	memcpy(packet.game.event.replay.compressedData, result.data(), result.size());
	SokuLib::DLL::ws2_32.sendto(sock, reinterpret_cast<char *>(&packet), sizeof(packet.game.event.replay), 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
}

void handlePacket(SokuLib::Packet &packet, sockaddr_in &addr)
{
	switch (packet.type) {
	case SokuLib::HELLO:
		packet.type = SokuLib::OLLEH;
		SokuLib::DLL::ws2_32.sendto(sock, reinterpret_cast<char *>(&packet), sizeof(packet.type), 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
		break;
	case SokuLib::INIT_REQUEST:
		handleInitRequest(packet, addr);
		break;
	case SokuLib::CHAIN:
		packet.chain.spectatorCount = 1;
		SokuLib::DLL::ws2_32.sendto(sock, reinterpret_cast<char *>(&packet), sizeof(packet.type), 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
		break;
	case SokuLib::CLIENT_GAME:
		handleClientGame(packet, addr);
		break;
	}
}

void networkLoop(void *)
{
	WSADATA WSAData;
	int len;
	struct sockaddr_in addr;

	if (SokuLib::DLL::ws2_32.WSAStartup(MAKEWORD(2, 2), &WSAData)) {
		puts(getLastSocketError().c_str());
		MessageBox(nullptr, getLastSocketError().c_str(), "Fatal error", MB_ICONERROR);
		abort();
	}
	sock = SokuLib::DLL::ws2_32.socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET) {
		puts(getLastSocketError().c_str());
		SokuLib::DLL::ws2_32.WSACleanup();
		MessageBox(nullptr, getLastSocketError().c_str(), "Fatal error", MB_ICONERROR);
		abort();
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(hostPort);
	addr.sin_addr.S_un.S_addr = 0;
	puts("Binding...");
	if (SokuLib::DLL::ws2_32.bind(sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr))) {
		puts(getLastSocketError().c_str());
		SokuLib::DLL::ws2_32.WSACleanup();
		MessageBox(nullptr, getLastSocketError().c_str(), "Fatal error", MB_ICONERROR);
		abort();
	}

	while (true) {
		SokuLib::Packet packet;

		addr.sin_family = AF_INET;
		addr.sin_port = htons(hostPort);
		addr.sin_addr.S_un.S_addr = 0;
		len = sizeof(addr);

		int size = SokuLib::DLL::ws2_32.recvfrom(sock, reinterpret_cast<char *>(&packet), sizeof(packet), 0, reinterpret_cast<sockaddr *>(&addr), &len);

		switch (SokuLib::sceneId) {
		case SokuLib::SCENE_LOADING:
		case SokuLib::SCENE_BATTLE:
		case SokuLib::SCENE_SELECT:
			break;
		default:
			continue;
		}

		if (SokuLib::mainMode != SokuLib::BATTLE_MODE_VSPLAYER)
			continue;

		if (size == -1) {
			puts("Error!");
			return;
		}
		handlePacket(packet, addr);
	}
}

void __fastcall KeymapManagerSetInputs(SokuLib::KeymapManager *This)
{
	(This->*s_origKeymapManager_SetInputs)();

	if (SokuLib::sceneId != SokuLib::SCENE_BATTLE)
		return;

	auto &mgr = SokuLib::getBattleMgr();
	SokuLib::BattleKeys *input;

	if (This == mgr.leftCharacterManager.keyManager->keymapManager)
		input = &replays[gameId].back().first;
	else if (mgr.rightCharacterManager.keyManager && This == mgr.rightCharacterManager.keyManager->keymapManager)
		input = &replays[gameId].back().second;
	else
		return;

	input->up = This->input.verticalAxis > 0;
	input->down = This->input.verticalAxis < 0;
	input->left = This->input.horizontalAxis < 0;
	input->right = This->input.horizontalAxis > 0;
	input->A = This->input.a;
	input->B = This->input.b;
	input->C = This->input.c;
	input->dash = This->input.d;
	input->AandB = This->input.changeCard;
	input->BandC = This->input.spellcard;
	//Practice::handleInput(*This);
}

int __fastcall CBattle_OnProcess(SokuLib::Battle *This) {
	// super
	mutex.lock();
	replays[gameId].emplace_back();

	int ret = (This->*s_origCBattle_Process)();

	loaded = false;
	finished |= SokuLib::getBattleMgr().leftCharacterManager.score == 2 || SokuLib::getBattleMgr().rightCharacterManager.score == 2;
	mutex.unlock();
	return ret;
}

int __fastcall CLoading_OnProcess(SokuLib::Loading *This) {
	// super
	int ret = (This->*s_origCLoading_Process)();

	if (!loaded) {
		gameId++;
		gameId %= replays.size();
		replays[gameId].clear();
	}
	finished = false;
	loaded = true;
	return ret;
}

// �ݒ胍�[�h
void LoadSettings(LPCSTR profilePath, LPCSTR parentPath) {
#ifdef _DEBUG
	FILE *_;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
#endif
	hostPort = GetPrivateProfileInt("Network", "Port", 10800, profilePath);
	_beginthread(networkLoop, 0, nullptr);
}

void hookFunctions() {
	DWORD old;

	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	//s_origCTitle_Process
	//	= SokuLib::union_cast<int (Title::*)()>(SokuLib::TamperDword(SokuLib::vtbl_CTitle + SokuLib::OFFSET_ON_PROCESS, reinterpret_cast<DWORD>(CTitle_OnProcess)));
	//s_origCBattleWatch_Process = SokuLib::union_cast<int (BattleWatch::*)()>(
	//	SokuLib::TamperDword(SokuLib::vtbl_CBattleWatch + SokuLib::OFFSET_ON_PROCESS, reinterpret_cast<DWORD>(CBattleWatch_OnProcess)));
	//s_origCLoadingWatch_Process = SokuLib::union_cast<int (LoadingWatch::*)()>(
	//	SokuLib::TamperDword(SokuLib::vtbl_CLoadingWatch + SokuLib::OFFSET_ON_PROCESS, reinterpret_cast<DWORD>(CLoadingWatch_OnProcess)));
	s_origCBattle_Process = SokuLib::TamperDword(&SokuLib::VTable_Battle.onProcess, CBattle_OnProcess);
	s_origCLoading_Process = SokuLib::TamperDword(&SokuLib::VTable_Loading.onProcess, CLoading_OnProcess);
	//s_origCBattleManager_Start = SokuLib::union_cast<int (SokuLib::BattleManager::*)()>(
	//	SokuLib::TamperDword(SokuLib::vtbl_CBattleManager + SokuLib::BATTLE_MGR_OFFSET_ON_SAY_START, reinterpret_cast<DWORD>(CBattleManager_Start)));
	//s_origCBattleManager_KO = SokuLib::union_cast<int (SokuLib::BattleManager::*)()>(
	//	SokuLib::TamperDword(SokuLib::vtbl_CBattleManager + SokuLib::BATTLE_MGR_OFFSET_ON_KO, reinterpret_cast<DWORD>(CBattleManager_KO)));
	/*s_origCBattleManager_Render = SokuLib::union_cast<int (SokuLib::BattleManager::*)()>(
		SokuLib::TamperDword(
			SokuLib::vtbl_CBattleManager + SokuLib::BATTLE_MGR_OFFSET_ON_RENDER,
			reinterpret_cast<DWORD>(CBattleManager_Render)
		)
	);*/
	realSendto   = SokuLib::TamperDword(&SokuLib::DLL::ws2_32.sendto, fakeSendto);
	realRecvfrom = SokuLib::TamperDword(&SokuLib::DLL::ws2_32.recvfrom, fakeRecvfrom);
	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);

	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	s_origKeymapManager_SetInputs = SokuLib::union_cast<void (SokuLib::KeymapManager::*)()>(SokuLib::TamperNearJmpOpr(0x40A45D, KeymapManagerSetInputs));
	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);
	FlushInstructionCache(GetCurrentProcess(), NULL, 0);
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return true;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	char profilePath[1024 + MAX_PATH];
	char profileParent[1024 + MAX_PATH];

	GetModuleFileName(hMyModule, profilePath, 1024);
	PathRemoveFileSpec(profilePath);
	strcpy(profileParent, profilePath);
	PathAppend(profilePath, "OfflineSpectator.ini");
	LoadSettings(profilePath, profileParent);

	//std::vector<byte> before{0x3c, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x22, 0x3c, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0};
	//std::vector<byte> result;

	//for (int i = -1; i < 10; i++) {
	//	ZUtils::compress(before.data(), before.size(), result, i);
	//	if (result.size() == 26)
	//		printf("%i\n", i);
	//	else
	//		printf("Size is %i\n", result.size());
	//}
	hookFunctions();
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}