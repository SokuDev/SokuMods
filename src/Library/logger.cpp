//
// Created by Gegel85 on 31/10/2020.
//

#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <shlwapi.h>
#include "logger.hpp"

static FILE *logFile;

void initLogger()
{
	FILE *_;

#ifdef _DEBUG
	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
#endif
	logFile = fopen(LOG_FILE, "w");
}

void logMessage(const char *msg)
{
	fwrite(msg, 1, strlen(msg), stdout);
	fwrite(msg, 1, strlen(msg), logFile);
}

void logMessagef(const char *format, ...)
{
	va_list list;

	va_start(list, format);
	vfprintf(stdout, format, list);
	va_end(list);
	va_start(list, format);
	vfprintf(logFile, format, list);
	va_end(list);
}