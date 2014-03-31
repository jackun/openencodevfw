#include "stdafx.h"
#include "log.h"

Logger::Logger(bool _log) : mLog(NULL), mWritelog(_log)
{
}

bool Logger::open()
{
	if(!mWritelog) return false;
	close();
	mLog = fopen("openencode.log", "w, ccs=UNICODE");
	return mLog != NULL;
}

void Logger::close()
{
	if(mLog) fclose(mLog);
	mLog = NULL;
}

Logger::~Logger()
{
	if(mLog) fclose(mLog);
}

void Logger::Log_internal(const wchar_t *psz_fmt, va_list arg)
{
	if(!mLog) open();

	if(mLog)
	{
		wchar_t msg[16000];
		memset(msg, 0, sizeof(msg));
		//FIXME fcker crashes, too small buffer, dafuq it is _s then??? murmur
		_vsnwprintf_s(msg, 16000, psz_fmt, arg);
		fwrite(msg, sizeof(wchar_t), wcslen(msg), mLog);
		fflush(mLog);
	}
}

void Logger::Log(const wchar_t *psz_fmt, ...)
{
	if(!mWritelog) return;
	va_list arg;
    va_start(arg, psz_fmt);
    Log_internal(psz_fmt, arg);
    va_end(arg);
}

void Logger::enableLog(bool b)
{
	mWritelog = b;
	if(!b) close();
}