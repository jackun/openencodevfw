#include "stdafx.h"
#include "log.h"

Logger::Logger()
{
	mLog = fopen("openencode.log", "w, ccs=UNICODE");
}

Logger::~Logger()
{
	if(mLog) fclose(mLog);
}

void Logger::Log_internal(const wchar_t *psz_fmt, va_list arg)
{
	if(!mLog) mLog = fopen("openencode.log", "w, ccs=UNICODE");

	if(mLog)
	{
		wchar_t msg[1024];
		memset(msg, 0, sizeof(msg));
		_vswprintf_c(msg, sizeof(msg), psz_fmt, arg);
		fwrite(msg, sizeof(wchar_t ), wcslen(msg), mLog);
		fflush(mLog);
	}
}

void Logger::Log(const wchar_t *psz_fmt, ...)
{
	va_list arg;
    va_start(arg, psz_fmt);
    Log_internal(psz_fmt, arg);
    va_end(arg);
}