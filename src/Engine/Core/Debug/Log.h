#pragma once

#include "Core/Object/Subsystem.h"

//=============================================================================
SE_NAMESPACE_BEGIN

class Log final : public Subsystem<Log>
{
public:
	// Log message type
	enum class Type
	{
		TRACE,					// Trace, also known as verbose logging
		DEBUG,					// Debug
		INFORMATION,			// Information
		WARNING,				// General warning
		PERFORMANCE_WARNING,	// Performance related warning
		COMPATIBILITY_WARNING,	// Compatibility related warning
		CRITICAL				// Critical
	};

	Log()
	{
		setValid(true);
	}
	
	/*
	*  @brief
	*    Receives an already formatted message for further processing
	*
	*  @param[in] type
	*    Log message type
	*  @param[in] file
	*    File as ASCII string
	*  @param[in] line
	*    Line number
	*  @param[in] message
	*    UTF-8 message
	*  @param[in] numberOfCharacters
	*    Number of characters inside the message, does not include the terminating zero character
	*
	*  @return
	*    "true" to request debug break, else "false"
	*/
	[[nodiscard]] inline bool Print(Type type, const char *file, uint32_t line, const char *format, ...)
	{
		bool requestDebugBreak = false;

		// Get the required buffer length, does not include the terminating zero character
		va_list vaList;
		va_start(vaList, format);
		const uint32_t textLength = static_cast<uint32_t>(vsnprintf(nullptr, 0, format, vaList));
		va_end(vaList);
		if ( 256 > textLength )
		{
			// Fast path: C-runtime stack

			// Construct the formatted text
			char formattedText[256];	// 255 +1 for the terminating zero character
			va_start(vaList, format);
			vsnprintf(formattedText, 256, format, vaList);
			va_end(vaList);

			// Internal processing
			requestDebugBreak = printInternal(type, file, line, formattedText);
		}
		else
		{
			// Slow path: Heap
			// -> No reused scratch buffer in order to reduce memory allocation/deallocations in here to not make things more complex and to reduce the mutex locked scope

			// Construct the formatted text
			char *formattedText = new char[textLength + 1];	// 1+ for the terminating zero character
			va_start(vaList, format);
			vsnprintf(formattedText, textLength + 1, format, vaList);
			va_end(vaList);

			// Internal processing
			requestDebugBreak = printInternal(type, file, line, formattedText);

			// Cleanup
			delete[] formattedText;
		}

		// Done
		return requestDebugBreak;
	}

private:
	[[nodiscard]] inline virtual bool printInternal(Type type, const char *file, uint32_t line, const char *message)
	{
		std::lock_guard<std::mutex> mutexLock(m_mutex);
		bool requestDebugBreak = false;

		// Construct the full UTF-8 message text
#if SE_DEBUG
		std::string fullMessage = "File \"" + std::string(file) + "\" | Line " + std::to_string(line) + " | " + std::string(typeToString(type)) + message;
#else
		std::string fullMessage = std::string(typeToString(type)) + message;
#endif
		if ( '\n' != fullMessage.back() )
		{
			fullMessage += '\n';
		}

		// Platform specific handling
#if SE_PLATFORM_WINDOWS
		{
			// Convert UTF-8 string to UTF-16
			std::wstring utf16Line;
			utf16Line.resize(static_cast<std::wstring::size_type>(::MultiByteToWideChar(CP_UTF8, 0, fullMessage.c_str(), static_cast<int>(fullMessage.size()), nullptr, 0)));
			::MultiByteToWideChar(CP_UTF8, 0, fullMessage.c_str(), static_cast<int>(fullMessage.size()), utf16Line.data(), static_cast<int>(utf16Line.size()));

			// Write into standard output stream
			if ( Type::CRITICAL == type )
				std::wcerr << utf16Line.c_str();
			else
				std::wcout << utf16Line.c_str();

			// On Microsoft Windows, ensure the output can be seen inside the Visual Studio output window as well
			::OutputDebugStringW(utf16Line.c_str());
			if ( Type::CRITICAL == type && ::IsDebuggerPresent() )
			{
				requestDebugBreak = true;
			}
		}
#elif SE_PLATFORM_ANDROID
		int prio = ANDROID_LOG_DEFAULT;
		switch ( type )
		{
		case Type::TRACE:
			prio = ANDROID_LOG_VERBOSE;
			break;

		case Type::DEBUG:
			prio = ANDROID_LOG_DEBUG;
			break;

		case Type::INFORMATION:
			prio = ANDROID_LOG_INFO;
			break;

		case Type::WARNING:
		case Type::PERFORMANCE_WARNING:
		case Type::COMPATIBILITY_WARNING:
			prio = ANDROID_LOG_WARN;
			break;

		case Type::CRITICAL:
			prio = ANDROID_LOG_ERROR;
			break;
		}
		__android_log_write(prio, "Sapphire", fullMessage.c_str());	// TODO(co) Might make sense to make the app-name customizable
#elif SE_PLATFORM_LINUX
	// Write into standard output stream
		if ( Type::CRITICAL == type )
		{
			std::cerr << fullMessage.c_str();
		}
		else
		{
			std::cout << fullMessage.c_str();
		}
#endif
		// Done
		return requestDebugBreak;
	}

	[[nodiscard]] inline const char* typeToString(Type type) const
	{
		switch ( type )
		{
		case Type::TRACE: return "Trace: ";
		case Type::DEBUG: return "Debug: ";
		case Type::INFORMATION: return "Information: ";
		case Type::WARNING: return "Warning: ";
		case Type::PERFORMANCE_WARNING: return "Performance warning: ";
		case Type::COMPATIBILITY_WARNING: return "Compatibility warning: ";
		case Type::CRITICAL: return "Critical: ";
		default: return "Unknown: ";
		}
	}	
	
	std::mutex m_mutex;
};

#define SE_LOG(type, format, ...) \
		do \
		{ \
			if (GetSubsystem<Log>().Print(Log::Type::type, __FILE__, static_cast<uint32_t>(__LINE__), format, ##__VA_ARGS__)) \
			{ \
				SE_DEBUG_BREAK; \
			} \
		} while (0);

SE_NAMESPACE_END
//=============================================================================