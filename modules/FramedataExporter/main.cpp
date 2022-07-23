#include <SokuLib.hpp>
#include <map>
#include <fstream>

#ifndef _DEBUG
#define puts(...)
#define printf(...)
#endif

using namespace SokuLib;

const int32_t minusTwo = -2;
std::map<int, std::string> textureNames;
int* (__fastcall *orig_loadTexture)(int self, int unused, int *ret, LPCSTR path, void *width, void *height);
void (*orig_loadPattern)(const char* charName, int palette, Map<int, v2::CharacterSequenceData*>* sequenceById, Deque<v2::CharacterSequenceData>* sequences, Vector<int>* textureIds);

// TextureManager::loadTexture called from: 0x467c43
// Soku2 can have other way to load texture, so you should test it.
int* __fastcall loadTexture(int self, int unused, int *ret, LPCSTR path, void *width, void *height) {
    printf("Loading %s\n", path);
    int* handle = orig_loadTexture(self, unused, ret, path, width, height);
    const char* name = strrchr(path, '/');
    if (name) textureNames[*handle] = name + 1;
    return handle;
}

static int countFrameData(Map<int, v2::CharacterSequenceData*>* sequenceById) {
    int i = 0;
    for (auto& pair : *sequenceById) {
        if (pair.first >= 1000) continue; // skip common
        auto sequence = pair.second;
        do { ++i; sequence = sequence->next; } while(sequence->previous);
    }
    return i;
}

static void writeSequence(std::ofstream& output, v2::CharacterSequenceData* data) {
    output.write((const char*)&data->unknown10, 5); // moveLock, actionLock, loop
    uint32_t count = data->frames.size();
    output.write((const char*)&count, 4);
    for (unsigned i = 0; i < data->frames.size(); ++i) {
        auto& frame = data->frames[i];
        uint32_t buffer = frame.texIndex;
        output.write((const char*)&buffer, 4);
        output.write((const char*)&frame.texOffset, 8); // texOffset and size
        output.write((const char*)&frame.offset, 6); // offset and duration
        output.write((const char*)&frame.renderGroup, 1);
        if (frame.renderGroup == 2) {
            buffer = frame.blendOptionsPtr->mode; --buffer;
            output.write((const char*)&buffer, 2);
            buffer = frame.blendOptionsPtr->color;
            output.write(((const char*)&buffer) + 3, 1);
            output.write(((const char*)&buffer) + 2, 1);
            output.write(((const char*)&buffer) + 1, 1);
            output.write(((const char*)&buffer), 1);
            buffer = frame.blendOptionsPtr->scale.x * 100;
            output.write((const char*)&buffer, 2);
            buffer = frame.blendOptionsPtr->scale.y * 100;
            output.write((const char*)&buffer, 2);
            buffer = frame.blendOptionsPtr->rotateX; // converting to integer
            output.write((const char*)&buffer, 2);
            buffer = frame.blendOptionsPtr->rotateY; // converting to integer
            output.write((const char*)&buffer, 2);
            buffer = frame.blendOptionsPtr->rotateZ; // converting to integer
            output.write((const char*)&buffer, 2);
        }

        output.write((const char*)&frame.damage, 30); // from damage to onGroundHitSet
        buffer = frame.onHitSpeed.x * 100;
        output.write((const char*)&buffer, 2);
        buffer = frame.onHitSpeed.y * 100;
        output.write((const char*)&buffer, 2);

        output.write((const char*)&frame.onHitSFX, 2);
        output.write((const char*)&frame.onHitFX, 2);
        // the next one is weird
        buffer = frame.attackType;
        output.write((const char*)&buffer, 2);
        buffer = frame.comboFlags;
        output.write((const char*)&buffer, 1);

        output.write((const char*)&frame.frameFlags, 8); // flags

        if (frame.collisionBox) {
            output.put('\x01');
            output.write((const char*)frame.collisionBox, 16);
        } else output.put('\x00');

        buffer = frame.hitBoxes.size();
        output.write((const char*)&buffer, 1);
        for (int i = 0; i < frame.hitBoxes.size(); ++i)
	    output.write((const char*)&frame.hitBoxes[i], 16);

        buffer = frame.attackBoxes.size();
        output.write((const char*)&buffer, 1);
        for (int i = 0; i < frame.attackBoxes.size(); ++i) {
            auto& box = frame.attackBoxes[i];
            output.write((const char*)&box, 16);
            if (frame.extraBoxes[i]) {
                output.put('\x01');
                output.write((const char*)frame.extraBoxes[i], 16);
            } else output.put('\x00');
        }

        output.write((const char*)&frame.extra1, 24);
        output.write((const char*)&frame.unknownA0, 6);
    }
}

// original addr: 0x4679e0
// called from: 0x46c0f7
void loadPattern(const char* charName, int palette, Map<int, v2::CharacterSequenceData*>* sequenceById, Deque<v2::CharacterSequenceData>* sequences, Vector<int>* textureIds) {
    printf("Loading %s.pat\n", charName);
    orig_loadPattern(charName, palette, sequenceById, sequences, textureIds);
    printf("Exporting %s.pat\n", charName);

    std::ofstream output("exported/" + std::string(charName)+".pat", std::ios::binary);

    output.put('\x05');
    uint32_t count = textureIds->size();
    output.write((const char*)&count, 2);
    for (int i = 0; i < textureIds->size(); i++) {
        auto id = (*textureIds)[i];
        const auto& textureName = textureNames[id];

        printf("%i (%s)\n", id, textureName.c_str());
        output.write(textureName.c_str(), textureName.size());
        for (unsigned i = textureName.size(); i < 128; ++i) output.put('\0');
    }

    count = countFrameData(sequenceById);
    output.write((const char*)&count, 4);
    for (auto& pair : *sequenceById) {
        if (pair.first >= 1000) continue; // skip common

        bool isFirst = true;
        auto sequence = pair.second;
        do {
	    if (isFirst) { output.write((const char*)&pair.first, 4); isFirst = false; }
	    else output.write((const char*)&minusTwo, 4);
            writeSequence(output, sequence);
            sequence = sequence->next;
        } while(sequence->previous != 0);
    }
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return memcmp(hash, SokuLib::targetHash, 16) == 0;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
	DWORD old;

#ifdef _DEBUG
	FILE *_;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	freopen_s(&_, "CONOUT$", "w", stderr);
#endif

	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	orig_loadTexture = (int* (__fastcall *)(int, int, int *, LPCSTR, void *, void *))SokuLib::TamperNearJmpOpr(0x467c43, loadTexture);
	orig_loadPattern = (void (*)(const char*, int, Map<int, v2::CharacterSequenceData*>*, Deque<v2::CharacterSequenceData>*, Vector<int>*))SokuLib::TamperNearJmpOpr(0x46c0f7, loadPattern);
	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);

	FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return TRUE;
}