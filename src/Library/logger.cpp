//
// Created by Gegel85 on 31/10/2020.
//

#include <cstdio>
#include <cstring>
#include <cstdarg>
#include "logger.hpp"

void initLogger()
{
	fclose(fopen(LOG_FILE, "w"));
}

void logMessage(const char *msg)
{
	FILE *fp = fopen(LOG_FILE, "a+");

	fwrite(msg, 1, strlen(msg), stdout);
	fwrite(msg, 1, strlen(msg), fp);
	fclose(fp);
}

void logMessagef(const char *format, ...)
{
	FILE *fp = fopen(LOG_FILE, "a+");
	va_list list;

	va_start(list, format);
	vfprintf(stdout, format, list);
	va_end(list);
	va_start(list, format);
	vfprintf(fp, format, list);
	va_end(list);
	fclose(fp);
}