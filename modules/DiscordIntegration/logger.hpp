//
// Created by Gegel85 on 31/10/2020.
//

#ifndef TOUHOUUNLDISCORDINTEGRATION_LOGGER_HPP
#define TOUHOUUNLDISCORDINTEGRATION_LOGGER_HPP

#define LOG_FILE "./DiscordIntegration.log"

void logMessagef(const char *format, ...);
void logMessage(const char *msg);
void initLogger();

#endif // TOUHOUUNLDISCORDINTEGRATION_LOGGER_HPP
