#pragma once

#include "Core/Object/Subsystem.h"

//=============================================================================
SE_NAMESPACE_BEGIN

class Assert final : public Subsystem<Assert>
{
public:
	Assert()
	{
		setValid(true);
	}

	[[nodiscard]] inline bool HandleAssert(const char *expression, const char *file, uint32_t line, const char *format, ...)
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
			requestDebugBreak = handleAssertInternal(expression, file, line, formattedText);
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
			requestDebugBreak = handleAssertInternal(expression, file, line, formattedText);

			// Cleanup
			delete[] formattedText;
		}

		// Done
		return requestDebugBreak;
	}
private:
	explicit Assert(const Assert&) = delete;
	Assert& operator=(const Assert&) = delete;

	[[nodiscard]] inline bool handleAssertInternal(const char *expression, const char *file, uint32_t line, const char *message)
	{
		std::lock_guard<std::mutex> mutexLock(m_mutex);
		bool requestDebugBreak = false;

		// Construct the full UTF-8 message text
		std::string fullMessage = "Assert message \"" + std::string(message) + "\" | Expression \"" + std::string(expression) + "\" | File \"" + std::string(file) + "\" | Line " + std::to_string(line);
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
			std::wcerr << utf16Line.c_str();

			// On Microsoft Windows, ensure the output can be seen inside the Visual Studio output window as well
			::OutputDebugStringW(utf16Line.c_str());
			if ( ::IsDebuggerPresent() )
			{
				requestDebugBreak = true;
			}
		}
#elif SE_PLATFORM_ANDROID
		__android_log_write(ANDROID_LOG_DEBUG, "Sapphire", fullMessage.c_str());
		requestDebugBreak = true;
#elif SE_PLATFORM_LINUX
	// Write into standard output stream
		std::cerr << fullMessage.c_str();
		requestDebugBreak = true;
#else
#error "Unsupported platform"
#endif
		return requestDebugBreak;
	}

	std::mutex m_mutex;
};

#if SE_DEBUG
#	define SE_ASSERT(expression, format, ...) \
			do \
			{ \
				if (!(expression) && GetSubsystem<Assert>().HandleAssert(#expression, __FILE__, static_cast<uint32_t>(__LINE__), format, ##__VA_ARGS__)) \
				{ \
					SE_DEBUG_BREAK; \
				} \
			} while (0);
#else
#	define SE_ASSERT(expression, format, ...) 
#endif

SE_NAMESPACE_END
//=============================================================================