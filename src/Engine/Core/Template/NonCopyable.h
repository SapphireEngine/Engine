#pragma once

//=============================================================================
SE_NAMESPACE_BEGIN

class NonCopyable
{
public:
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable(NonCopyable&&) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;
	NonCopyable& operator=(NonCopyable&&) = delete;
	~NonCopyable() = default;
protected:
	NonCopyable() = default;
};

SE_NAMESPACE_END
//=============================================================================