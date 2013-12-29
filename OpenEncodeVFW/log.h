#ifndef _LOGGER
#define _LOGGER

class Logger
{
private:
	FILE* mLog;
	bool mWritelog;
public:
	Logger(bool _log);
	~Logger();
	void enableLog(bool b);
	bool open();
	void close();

	void Log_internal(const wchar_t *psz_fmt, va_list arg);
	void Log(const wchar_t *psz_fmt, ...);
};

#endif