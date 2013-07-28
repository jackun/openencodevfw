#ifndef _LOGGER
#define _LOGGER

class Logger
{
private:
	FILE* mLog;
public:
	Logger();
	~Logger();

	void Log_internal(const wchar_t *psz_fmt, va_list arg);
	void Log(const wchar_t *psz_fmt, ...);
};

#endif