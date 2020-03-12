#include "stdafx.h"
#include "Log.h"

//=============================================================================
SE_NAMESPACE_BEGIN
#define LOG_PREAMBLE_SIZE (56 + MAX_THREAD_NAME_LENGTH + FILENAME_NAME_LENGTH_LOG)

static Log* pLogger;
static bool gOnce = true;

stl::string GetTimeStamp()
{
	time_t sysTime;
	time(&sysTime);
	stl::string dateTime = ctime(&sysTime);
	stl::replace(dateTime.begin(), dateTime.end(), '\n', ' ');
	return dateTime;
}

// Returns the part of the path after the last / or \ (if any).
const char* get_filename(const char* path)
{
	for ( auto ptr = path; *ptr; ++ptr )
	{
		if ( *ptr == '/' || *ptr == '\\' )
		{
			path = ptr + 1;
		}
	}
	return path;
}

// Default callback
void log_write(void * user_data, const stl::string & message)
{
	FileStream* fh = (FileStream*)user_data;
	ASSERT(fh);

	fsWriteToStreamLine(fh, message.c_str());
	fsFlushStream(fh);
}

// Close callback
void log_close(void * user_data)
{
	FileStream* fh = (FileStream*)user_data;
	ASSERT(fh);
	fsCloseStream(fh);
}

// Flush callback
void log_flush(void * user_data)
{
	FileStream* fh = (FileStream*)user_data;
	ASSERT(fh);

	fsFlushStream(fh);
}

void Log::Init(LogLevel level /* = LogLevel::eALL */)
{
	pLogger = conf_new(Log, level);
	pLogger->mLogMutex.Init();
}

void Log::Exit()
{
	pLogger->mLogMutex.Destroy();
	conf_delete(pLogger);
	pLogger = NULL;
}

Log::LogScope::LogScope(uint32_t log_level, const char * file, int line, const char * format, ...)
	: mFile(file)
	, mLine(line)
	, mLevel(log_level)
{
	const unsigned BUFFER_SIZE = 4096;
	char           buf[BUFFER_SIZE];
	va_list arglist;
	va_start(arglist, format);
	vsprintf_s(buf, BUFFER_SIZE, format, arglist);
	va_end(arglist);
	mMessage = buf;

	// Write to log and update indentation
	Log::Write(mLevel, "{ " + mMessage, mFile, mLine);
	{
		MutexLock lock{ pLogger->mLogMutex };
		++pLogger->mIndentation;
	}
}

Log::LogScope::~LogScope()
{
	// Update indentation and write to log
	{
		MutexLock lock{ pLogger->mLogMutex };
		--pLogger->mIndentation;
	}
	Log::Write(mLevel, "} " + mMessage, mFile, mLine);
}

void Log::SetLevel(LogLevel level)
{
	pLogger->mLogLevel = level;
}
void Log::SetQuiet(bool bQuiet)
{
	pLogger->mQuietMode = bQuiet;
}
void Log::SetTimeStamp(bool bEnable)
{
	pLogger->mRecordTimestamp = bEnable;
}
void Log::SetRecordingFile(bool bEnable)
{
	pLogger->mRecordFile = bEnable;
}
void Log::SetRecordingThreadName(bool bEnable)
{
	pLogger->mRecordThreadName = bEnable;
}

// Gettors
uint32_t Log::GetLevel()
{
	return pLogger->mLogLevel;
}
stl::string Log::GetLastMessage()
{
	return pLogger->mLastMessage;
}
bool Log::IsQuiet()
{
	return pLogger->mQuietMode;
}
bool Log::IsRecordingTimeStamp()
{
	return pLogger->mRecordTimestamp;
}
bool Log::IsRecordingFile()
{
	return pLogger->mRecordFile;
}
bool Log::IsRecordingThreadName()
{
	return pLogger->mRecordThreadName;
}

void Log::AddFile(const char * filename, FileMode file_mode, LogLevel log_level)
{
	if ( filename == NULL )
		return;

	PathHandle logFileDirectory = fsCopyLogFileDirectoryPath();
	PathHandle path = fsAppendPathComponent(logFileDirectory, filename);
	FileStream* fh = fsOpenFile(path, file_mode);
	if ( fh )//If the File Exists
	{

		// AddCallback will try to acquire mutex

		AddCallback(fsGetPathAsNativeString(path), log_level, fh, log_write, log_close, log_flush);

		{
			MutexLock lock{ pLogger->mLogMutex }; // scope lock as Write will try to acquire mutex

			// Header
			stl::string header;
			if ( pLogger->mRecordTimestamp )
				header += "date       time     ";
			if ( pLogger->mRecordThreadName )
				header += "[thread name/id ]";
			if ( pLogger->mRecordFile )
				header += "                   file:line  ";
			header += "  v |\n";
			fsWriteToStream(fh, header.c_str(), header.size());
			fsFlushStream(fh);
			//file->Write(header.c_str(), (unsigned)header.size());
			//file->Flush();
		}

		Write(LogLevel::eINFO, "Opened log file " + stl::string{ filename }, __FILE__, __LINE__);
	}
	else
	{
		Write(LogLevel::eERROR, "Failed to create log file " + stl::string{ filename }, __FILE__, __LINE__); // will try to acquire mutex
	}
}

void Log::AddCallback(const char * id, uint32_t log_level, void * user_data, log_callback_t callback, log_close_t close, log_flush_t flush)
{
	MutexLock lock{ pLogger->mLogMutex };
	if ( !CallbackExists(id) )
	{
		pLogger->mCallbacks.emplace_back(LogCallback{ id, user_data, callback, close, flush, log_level });
	}
	else
		close(user_data);
}

void Log::Write(uint32_t level, const stl::string & message, const char * filename, int line_number)
{
	static stl::pair<uint32_t, stl::string> logLevelPrefixes[] =
	{
		stl::pair<uint32_t, stl::string>{ LogLevel::eWARNING, stl::string{ "WARN" } },
		stl::pair<uint32_t, stl::string>{ LogLevel::eINFO, stl::string{ "INFO" } },
		stl::pair<uint32_t, stl::string>{ LogLevel::eDEBUG, stl::string{ " DBG" } },
		stl::pair<uint32_t, stl::string>{ LogLevel::eERROR, stl::string{ " ERR" } }
	};

	stl::string log_level_strings[LEVELS_LOG];
	uint32_t log_levels[LEVELS_LOG];
	uint32_t log_level_count = 0;

	// Check flags
	for ( uint32_t i = 0; i < sizeof(logLevelPrefixes) / sizeof(logLevelPrefixes[0]); ++i )
	{
		const stl::pair<uint32_t, stl::string>* it = &logLevelPrefixes[i];
		if ( it->first & level )
		{
			log_level_strings[log_level_count] = it->second + "| ";
			log_levels[log_level_count] = it->first;
			++log_level_count;
		}
	}

	bool do_once = false;
	{
		MutexLock lock{ pLogger->mLogMutex }; // scope lock as stack frames from calling AddInitialLogFile will attempt to lock mutex
		do_once = gOnce;
		gOnce = false;
	}
	if ( do_once )
		AddInitialLogFile();

	MutexLock lock{ pLogger->mLogMutex };
	pLogger->mLastMessage = message;

	char preamble[LOG_PREAMBLE_SIZE] = { 0 };
	WritePreamble(preamble, LOG_PREAMBLE_SIZE, filename, line_number);

	// Prepare indentation
	stl::string indentation;
	indentation.resize(pLogger->mIndentation * INDENTATION_SIZE_LOG);
	memset(indentation.begin(), ' ', indentation.size());

	// Log for each flag
	for ( uint32_t i = 0; i < log_level_count; ++i )
	{
		stl::string formattedMessage = preamble + log_level_strings[i] + indentation + message;

		if ( pLogger->mQuietMode )
		{
			if ( level & LogLevel::eERROR )
				_PrintUnicodeLine(formattedMessage, true);
		}
		else
		{
			_PrintUnicodeLine(formattedMessage, level & LogLevel::eERROR);
		}
	}

	for ( LogCallback & callback : pLogger->mCallbacks )
	{
		// Log for each flag
		for ( uint32_t i = 0; i < log_level_count; ++i )
		{
			if ( callback.mLevel & log_levels[i] )
				callback.mCallback(callback.mUserData, preamble + log_level_strings[i] + indentation + message);
		}
	}
}

void Log::WriteRaw(uint32_t level, const stl::string & message, bool error)
{
	bool do_once = false;
	{
		MutexLock lock{ pLogger->mLogMutex }; // scope lock as stack frames from calling AddInitialLogFile will attempt to lock mutex
		do_once = gOnce;
		gOnce = false;
	}
	if ( do_once )
		AddInitialLogFile();

	MutexLock lock{ pLogger->mLogMutex };

	pLogger->mLastMessage = message;

	if ( pLogger->mQuietMode )
	{
		if ( error )
			_PrintUnicode(message, true);
	}
	else
		_PrintUnicode(message, error);

	for ( LogCallback & callback : pLogger->mCallbacks )
	{
		if ( callback.mLevel & level )
			callback.mCallback(callback.mUserData, message);
	}
}

void Log::AddInitialLogFile()
{

	// Add new file with executable name

	const char *extension = ".log";
	const size_t extensionLength = strlen(extension);

	char exeFileName[256];
	fsGetExecutableName(exeFileName, 256 - extensionLength);

	// Minimum length check
	if ( exeFileName[0] == 0 || exeFileName[1] == 0 )
	{
		strncpy(exeFileName, "Log", 3);
	}
	strncat(exeFileName, extension, extensionLength);

	AddFile(exeFileName, FM_WRITE_BINARY, LogLevel::eALL);
}

void Log::WritePreamble(char * buffer, uint32_t buffer_size, const char * file, int line)
{
	time_t  t = time(NULL);
	tm time_info;
#ifdef _WIN32
	localtime_s(&time_info, &t);
#else
	localtime_r(&t, &time_info);
#endif

	uint32_t pos = 0;
	// Date and time
	if ( pLogger->mRecordTimestamp && pos < buffer_size )
	{
		pos += snprintf(buffer + pos, buffer_size - pos, "%04d-%02d-%02d ",
			1900 + time_info.tm_year, 1 + time_info.tm_mon, time_info.tm_mday);
		pos += snprintf(buffer + pos, buffer_size - pos, "%02d:%02d:%02d ",
			time_info.tm_hour, time_info.tm_min, time_info.tm_sec);
	}

	if ( pLogger->mRecordThreadName && pos < buffer_size )
	{
		char thread_name[MAX_THREAD_NAME_LENGTH + 1] = { 0 };
		Thread::GetCurrentThreadName(thread_name, MAX_THREAD_NAME_LENGTH + 1);

		// No thread name
		if ( thread_name[0] == 0 )
			snprintf(thread_name, MAX_THREAD_NAME_LENGTH + 1, "NoName");

		pos += snprintf(buffer + pos, buffer_size - pos, "[%-15s]", thread_name);
	}

	// File and line
	if ( pLogger->mRecordFile && pos < buffer_size )
	{
		file = get_filename(file);

		char shortened_filename[FILENAME_NAME_LENGTH_LOG + 1];
		snprintf(shortened_filename, FILENAME_NAME_LENGTH_LOG + 1, "%s", file);
		pos += snprintf(buffer + pos, buffer_size - pos, " %22s:%-5u ", shortened_filename, line);
	}
}

bool Log::CallbackExists(const char * id)
{
	for ( const LogCallback & callback : pLogger->mCallbacks )
	{
		if ( callback.mID == id )
			return true;
	}

	return false;
}

Log::Log(LogLevel level)
	: mLogLevel(level)
	, mIndentation(0)
	, mQuietMode(false)
	, mRecordTimestamp(true)
	, mRecordFile(true)
	, mRecordThreadName(true)
{
	Thread::SetMainThread();
	Thread::SetCurrentThreadName("MainThread");
}

Log::~Log()
{
	for ( LogCallback & callback : mCallbacks )
	{
		if ( callback.mClose )
			callback.mClose(callback.mUserData);
	}

	mCallbacks.clear();
}

stl::string ToString(const char* format, ...)
{
	const unsigned BUFFER_SIZE = 4096;
	char           buf[BUFFER_SIZE];

	va_list arglist;
	va_start(arglist, format);
	vsprintf_s(buf, BUFFER_SIZE, format, arglist);
	va_end(arglist);

	return stl::string(buf);
}

#if SE_PLATFORM_WINDOWS

void outputLogString(const char* pszStr)
{
	OutputDebugStringA(pszStr);
	OutputDebugStringA("\n");
}

void _OutputDebugStringV(const char* str, va_list args)
{
#ifdef _DEBUG
	const unsigned BUFFER_SIZE = 4096;
	char           buf[BUFFER_SIZE];

	vsprintf_s(buf, BUFFER_SIZE, str, args);

	OutputDebugStringA(buf);
	OutputDebugStringA("\n");
#endif
}

void _OutputDebugString(const char* str, ...)
{
	va_list arglist;
	va_start(arglist, str);
	_OutputDebugStringV(str, arglist);
	va_end(arglist);
}

void _FailedAssert(const char* file, int line, const char* statement)
{
	static bool debug = true;

	if ( debug )
	{
		WCHAR str[1024];
		WCHAR message[1024];
		WCHAR wfile[1024];
		mbstowcs(message, statement, 1024);
		mbstowcs(wfile, file, 1024);
		wsprintfW(str, L"Failed: (%s)\n\nFile: %s\nLine: %d\n\n", message, wfile, line);

		if ( IsDebuggerPresent() )
		{
			wcscat(str, L"Debug?");
			int res = MessageBoxW(NULL, str, L"Assert failed", MB_YESNOCANCEL | MB_ICONERROR);
			if ( res == IDYES )
			{
#if _MSC_VER >= 1400
				__debugbreak();
#else
				_asm int 0x03;
#endif
			}
			else if ( res == IDCANCEL )
			{
				debug = false;
			}
		}
		else
		{
			wcscat(str, L"Display more asserts?");
			if ( MessageBoxW(NULL, str, L"Assert failed", MB_YESNO | MB_ICONERROR | MB_DEFBUTTON2) != IDYES )
			{
				debug = false;
			}
		}
	}
}

void _PrintUnicode(const stl::string& str, bool error)
{
	// If the output stream has been redirected, use fprintf instead of WriteConsoleW,
	// though it means that proper Unicode output will not work
	FILE* out = error ? stderr : stdout;
	if ( !_isatty(_fileno(out)) )
		fprintf(out, "%s\n", str.c_str());
	else
	{
		if ( error )
			printf("%s\n", str.c_str());    // use this for now because WriteCosnoleW sometimes cause blocking
		else
			printf("%s\n", str.c_str());
	}

	outputLogString(str.c_str());
}

void _PrintUnicodeLine(const stl::string& str, bool error)
{
	_PrintUnicode(str, error);
}
#endif

SE_NAMESPACE_END
//=============================================================================