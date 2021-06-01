//
// Created by PinkySmile on 18/04/2021.
//

#define _CRT_SECURE_NO_WARNINGS 1
#include <Windows.h>
#include <stdbool.h>
#include <stdio.h>
#include <Shlwapi.h>

unsigned short defaultDecks[20][20] = {
	{1, 1, 1, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 208, 208, 208},
	{ 0, 1, 1, 1, 1, 100, 100, 101, 101, 102, 103, 200, 200, 200, 202, 202, 208, 208, 208, 208 },
	{ 0, 1, 1, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 201, 203, 203 },
	{ 0, 1, 1, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 202, 202, 202, 203, 203 },
	{ 100, 100, 101, 101, 102, 102, 103, 103, 104, 104, 200, 200, 200, 200, 201, 201, 201, 202, 202, 202 },
	{ 1, 1, 1, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 201, 203, 203 },
	{ 1, 1, 1, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 201, 203, 203 },
	{ 0, 0, 0, 1, 1, 1, 1, 100, 101, 102, 103, 200, 200, 200, 201, 201, 201, 202, 202, 202 },
	{ 1, 1, 1, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 201, 203, 203 },
	{ 0, 0, 1, 1, 100, 100, 101, 101, 102, 102, 103, 200, 200, 200, 201, 201, 201, 202, 202, 202 },
	{ 0, 0, 0, 1, 1, 1, 1, 100, 101, 102, 103, 200, 200, 200, 202, 202, 202, 206, 206, 206 },
	{ 0, 0, 1, 1, 1, 1, 100, 100, 101, 102, 103, 200, 200, 200, 201, 201, 201, 205, 205, 205 },
	{ 1, 1, 1, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 202, 202, 202, 205, 205, 205 },
	{ 0, 0, 0, 1, 1, 1, 1, 100, 101, 102, 103, 200, 200, 200, 201, 201, 201, 202, 202, 202 },
	{ 0, 0, 0, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 201, 201, 201, 202, 202, 202 },
	{ 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 201, 201, 203, 203, 203, 203 },
	{ 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 201, 201, 202, 202, 202, 202 },
	{ 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 202, 202, 202, 202, 204, 204, 204, 204 },
	{ 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 203, 203, 203, 203, 213, 213, 213, 213 },
	{ 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 201, 201, 204, 204, 204, 204 }
};

static void exploreFile(wchar_t *buffer, size_t bufferSize, const wchar_t *title, const wchar_t *basePath)
{
	OPENFILENAMEW ofn;

	memset(buffer, 0, bufferSize);
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = buffer;
	ofn.nMaxFile = bufferSize;
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.lpstrFilter = L"th123.exe\0th123.exe\0\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nFilterIndex = 0;
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrInitialDir = basePath;
	ofn.lpstrTitle = title;
	ofn.lpstrDefExt = NULL;
	GetOpenFileNameW(&ofn);
}

int compare(const void *a, const void *b)
{
	return *(unsigned short *)a - *(unsigned short *)b;
}

bool allDecksDefault(unsigned short (*decks)[4][20], unsigned i)
{
	if (i >= 20)
		return false;
	for (int j = 0; j < 4; j++) {
		qsort(decks[i][j], 20, 2, compare);
		if (memcmp(defaultDecks[i], decks[i][j], sizeof(defaultDecks[i])) != 0)
			return false;
	}
	return true;
}

void convertDeck(const wchar_t *path)
{
	wchar_t buffer[MAX_PATH];
	unsigned char length;
	unsigned short cards[255];
	FILE *json;
	FILE *profile;
	unsigned short (*decks)[4][20] = NULL;
	int size;

	wcscpy(buffer, path);
	*wcsrchr(buffer, '.') = 0;
	wcscat(buffer, L".json");
	wprintf(L"Loading decks from profile file %s to %s.\n", path, buffer);

	json = _wfopen(buffer, L"r");
	if (json) {
		fclose(json);
		wprintf(L"%s already exists... Ignoring.\n", buffer);
		return;
	}

	profile = _wfopen(path, L"r");
	if (!profile) {
		wprintf(L"Can't open %s for reading %S\n", path, strerror(errno));
		return;
	}

	json = _wfopen(buffer, L"w");
	if (!json) {
		fclose(profile);
		wprintf(L"Can't open %s for writing %S\n", buffer, strerror(errno));
		return;
	}

	fseek(profile, 106, SEEK_SET);
	for (size = 1; !feof(profile); size++) {
		decks = realloc(decks, sizeof(*decks) * size);
		for (int k = 0; k < 4; k++) {
			fread(&length, sizeof(length), 1, profile);
			fread(cards, sizeof(*cards), length, profile);
			for (int j = length; j < 20; j++)
				cards[j] = 21;
			memcpy(decks[size - 1][k], cards, 40);
		}
	}
	fclose(profile);

	fwrite("{", 1, 1, json);

	const char *deckNames[4] = {
		"yorokobi",
		"ikari",
		"ai",
		"tanoshii"
	};
	bool first = true;
	bool first2 = true;
	unsigned i = 0;

	size -= 2;
	if (size > 20)
		size -= 2;
	printf("There are %i characters...\n", size);
	while (size--) {
		if (allDecksDefault(decks, i)) {
			printf("Character %i has all default decks\n", i);
			i++;
			if (i == 20)
				i += 2;
			continue;
		}
		fprintf(json, "%s\n\t\"%i\": [", first2 ? "" : ",", i);
		first2 = false;
		first = true;
		for (int j = 0; j < 4; j++) {
			qsort(decks[i][j], 20, 2, compare);
			if (i < 20 && memcmp(defaultDecks[i], decks[i][j], sizeof(defaultDecks[i])) == 0)
				continue;
			fprintf(json, "%s\n\t\t{\n\t\t\t\"name\": \"%s\",\n\t\t\t\"cards\": [", first ? "" : ",", deckNames[j]);
			first = false;
			for (int k = 0; k < 20; k++)
				fprintf(json, "%s%i", k == 0 ? "" : ", ", decks[i][j][k]);
			fwrite("]\n\t\t}", 1, 5, json);
		}
		fwrite("\n\t]", 1, 3, json);
		i++;
		if (i == 20)
			i += 2;
	}
	fwrite("\n}", 1, 2, json);
	fclose(json);
	free(decks);
	decks = NULL;
}

LPWSTR GetLastErrorAsString(DWORD errorMessageID)
{
	if (errorMessageID == 0) {
		return NULL;
	}

	LPWSTR messageBuffer = NULL;

	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorMessageID,
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
		(LPWSTR)&messageBuffer,
		0,
		NULL
	);

	return messageBuffer;
}


int convertDecks(const wchar_t *path)
{
	wchar_t buffer[MAX_PATH];
	wchar_t buffer2[MAX_PATH];
	WIN32_FIND_DATAW data;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	wcscpy(buffer, path);
	wcscat(buffer, L"\\profile\\*.pf");
	wprintf(L"Looking for files %s\n", buffer);
	hFind = FindFirstFileW(buffer, &data);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		DWORD err = GetLastError();
		wchar_t buffer3[1024];

		swprintf(buffer3, 1024, L"FindFirstFileW: %s (%lu)\n", GetLastErrorAsString(err), err);
		printf("Error in FindFirstFileW : %lu\n", err);
		MessageBoxW(NULL, buffer3, L"Error", MB_ICONERROR);
		return EXIT_FAILURE;
	}

	wcscpy(buffer, path);
	wcscat(buffer, L"\\profile\\");

	do {
		wcscpy(buffer2, buffer);
		wcscat(buffer2, data.cFileName);
		convertDeck(buffer2);
	} while (FindNextFileW(hFind, &data));
	MessageBoxA(NULL, "All profiles has been treated !", "All done", MB_ICONINFORMATION);
	return EXIT_SUCCESS;
}

int main()
{
	wchar_t buffer[MAX_PATH];

	exploreFile(buffer, sizeof buffer, L"Select game path", L".");
	if (!*buffer || !wcsstr(buffer, L"th123.exe") || wcsstr(buffer, L"th123.exe")[strlen("th123.exe")]) {
		MessageBoxA(NULL, "Please specify th123.exe location", "Invalid location", MB_ICONERROR);
		return EXIT_FAILURE;
	}
	PathRemoveFileSpecW(buffer);
	return convertDecks(buffer);
}