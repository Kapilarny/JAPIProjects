#ifndef JAPI_REWRITE_TESTPLUGIN_PLUGIN_H
#define JAPI_REWRITE_TESTPLUGIN_PLUGIN_H

#define EXPORT extern "C" __declspec(dllexport)

#include <JoJoAPI.h>

EXPORT JAPIModMeta __stdcall GetModMeta();
EXPORT void __stdcall ModInit();

#define JFATAL(fmt, ...) JAPI_LogMessage(JAPI_LOG_LEVEL_FATAL, fmt, ##__VA_ARGS__)
#define JERROR(fmt, ...) JAPI_LogMessage(JAPI_LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define JDEBUG(fmt, ...) JAPI_LogMessage(JAPI_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define JWARN(fmt, ...) JAPI_LogMessage(JAPI_LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define JINFO(fmt, ...) JAPI_LogMessage(JAPI_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define JTRACE(fmt, ...) JAPI_LogMessage(JAPI_LOG_LEVEL_TRACE, fmt, ##__VA_ARGS__)

#endif //JAPI_REWRITE_TESTPLUGIN_PLUGIN_H