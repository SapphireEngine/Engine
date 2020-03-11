#pragma once

#include "Platform/Thread.h"
#include "Platform/FileSystem.h"

//=============================================================================
SE_NAMESPACE_BEGIN

#ifndef FILENAME_NAME_LENGTH_LOG
#define FILENAME_NAME_LENGTH_LOG 23
#endif

#ifndef INDENTATION_SIZE_LOG
#define INDENTATION_SIZE_LOG 4
#endif

#ifndef LEVELS_LOG
#define LEVELS_LOG 6
#endif

#define CONCAT_STR_LOG_IMPL(a, b) a ## b
#define CONCAT_STR_LOG(a, b) CONCAT_STR_LOG_IMPL(a, b)

#ifndef ANONIMOUS_VARIABLE_LOG
#define ANONIMOUS_VARIABLE_LOG(str) CONCAT_STR_LOG(str, __LINE__)
#endif

// If you add more levels don't forget to change LOG_LEVELS macro to the actual number of levels
enum LogLevel
{
	eNONE = 0,
	eRAW = 1,
	eDEBUG = 2,
	eINFO = 4,
	eWARNING = 8,
	eERROR = 16,
	eALL = ~0
};


typedef void(*log_callback_t)(void * user_data, const eastl::string & message);
typedef void(*log_close_t)(void * user_data);
typedef void(*log_flush_t)(void * user_data);

/// Logging subsystem.
class Log
{
public:
	struct LogScope
	{
		LogScope(uint32_t log_level, const char * file, int line, const char * format, ...);
		~LogScope();

		eastl::string mMessage;
		const char * mFile;
		int mLine;
		uint32_t mLevel;
	};

	Log(LogLevel level = LogLevel::eALL);
	~Log();

	static void Init(LogLevel level = LogLevel::eALL);
	static void Exit();

	static void SetLevel(LogLevel level);
	static void SetQuiet(bool bQuiet);
	static void SetTimeStamp(bool bEnable);
	static void SetRecordingFile(bool bEnable);
	static void SetRecordingThreadName(bool bEnable);

	static uint32_t        GetLevel();
	static eastl::string   GetLastMessage();
	static bool            IsQuiet();
	static bool            IsRecordingTimeStamp();
	static bool            IsRecordingFile();
	static bool            IsRecordingThreadName();

	static void AddFile(const char * filename, FileMode file_mode, LogLevel log_level);
	static void AddCallback(const char * id, uint32_t log_level, void * user_data, log_callback_t callback, log_close_t close = nullptr, log_flush_t flush = nullptr);

	static void Write(uint32_t level, const eastl::string& message, const char * filename, int line_number);
	static void WriteRaw(uint32_t level, const eastl::string& message, bool error = false);

private:
	static void AddInitialLogFile();
	static void WritePreamble(char * buffer, uint32_t buffer_size, const char * file, int line);
	static bool CallbackExists(const char * id);

	// Singleton
	Log(const Log &) = delete;
	Log(Log &&) = delete;
	Log & operator=(const Log &) = delete;
	Log & operator=(Log &&) = delete;

	struct LogCallback
	{
		LogCallback(const eastl::string & id, void * user_data, log_callback_t callback, log_close_t close, log_flush_t flush, uint32_t level)
			: mID(id)
			, mUserData(user_data)
			, mCallback(callback)
			, mClose(close)
			, mFlush(flush)
			, mLevel(level)
		{
		}

		eastl::string mID;
		void * mUserData;
		log_callback_t mCallback;
		log_close_t mClose = nullptr;
		log_flush_t mFlush = nullptr;
		uint32_t mLevel;
	};

	eastl::vector<LogCallback> mCallbacks;
	/// Mutex for threaded operation.
	Mutex           mLogMutex;
	eastl::string   mLastMessage;
	uint32_t        mLogLevel;
	uint32_t        mIndentation;
	bool            mQuietMode;
	bool            mRecordTimestamp;
	bool            mRecordFile;
	bool            mRecordThreadName;
};

stl::string ToString(const char* formatString, ...);

void _FailedAssert(const char* file, int line, const char* statement);
void _OutputDebugString(const char* str, ...);
void _OutputDebugStringV(const char* str, va_list args);

void _PrintUnicode(const eastl::string& str, bool error = false);
void _PrintUnicodeLine(const eastl::string& str, bool error = false);

#if _MSC_VER >= 1400
// To make MSVC 2005 happy
#pragma warning(disable : 4996)
#define assume(x) __assume(x)
#define no_alias __declspec(noalias)
#else
#define assume(x)
#define no_alias
#endif

#ifdef _DEBUG
#define ASSERT(b) assume(b)
#else
#define ASSERT(b)
#endif

// Usage: LOGF(LogLevel::eINFO | LogLevel::eDEBUG, "Whatever string %s, this is an int %d", "This is a string", 1)
#define LOGF(log_level, ...) Log::Write((log_level), ToString(__VA_ARGS__), __FILE__, __LINE__)

SE_NAMESPACE_END
//=============================================================================