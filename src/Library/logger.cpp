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

	logFile = fopen(LOG_FILE, "w");
	fclose(logFile);
#ifdef _DEBUG
	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
#endif
}

void logMessage(const char *msg)
{
#ifdef _DEBUG
	fwrite(msg, 1, strlen(msg), stdout);
#endif
	logFile = fopen(LOG_FILE, "a");
	fwrite(msg, 1, strlen(msg), logFile);
	fclose(logFile);
}

void logMessagef(const char *format, ...)
{
	va_list list;

#ifdef _DEBUG
	va_start(list, format);
	vfprintf(stdout, format, list);
	va_end(list);
#endif
	va_start(list, format);
	logFile = fopen(LOG_FILE, "a");
	vfprintf(logFile, format, list);
	fclose(logFile);
	va_end(list);
}