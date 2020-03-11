#pragma once

#include "Core/Debug/BaseErrorHanding.h"
#include "Core/Template/NonCopyable.h"

//=============================================================================
SE_NAMESPACE_BEGIN

template <typename T>
class Subsystem : NonCopyable
{
public:
	template<class ...Args>
	static bool Create(Args&& ...args)
	{
		if( IsValid() )
		{
			CriticalError("Trying to create an already created module.");
			return false;
		}

		instance() = new T(stl::forward<Args>(args)...);
		isCreate() = true;
		return IsValid();
	}

	template<typename SubClass, class ...Args>
	static bool Create(Args&& ...args)
	{
		if( IsValid() )
		{
			CriticalError("Trying to create an already created module.");
			return false;
		}

		instance() = static_cast<T*>(new SubClass(stl::forward<Args>(args)...));
		isCreate() = true;
		return IsValid();
	}

	static void Destroy()
	{
		SafeDelete(instance());
		valid() = false;
	}

	static T& Get()
	{
		if( !IsValid() )
			CriticalError("Trying to access a module but it hasn't been created up yet.");

		return *instance();
	}

	static const bool IsValid()
	{
		return valid() && isCreate();
	}

protected:
	Subsystem() = default;
	~Subsystem() = default;

	static T*& instance()
	{
		static T* inst = nullptr;
		return inst;
	}

	static bool& valid()
	{
		static bool inst = false;
		return inst;
	}

	static bool& isCreate()
	{
		static bool inst = false;
		return inst;
	}

	static void setValid(bool bvalid)
	{
		valid() = bvalid;
	}
};

template <typename T>
inline T& GetSubsystem()
{
	return T::Get();
}

SE_NAMESPACE_END
//=============================================================================