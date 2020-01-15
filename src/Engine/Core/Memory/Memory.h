#pragma once

#include "Core/Object/Subsystem.h"
#include "Core/Memory/IAllocator.h"

//=============================================================================
SE_NAMESPACE_BEGIN

class Memory : public Subsystem<Memory>
{
public:
	Memory();
	~Memory();

	IAllocator* DefaultAlloc() { return m_defaultAlloc; }


private:
	IAllocator *m_defaultAlloc = nullptr;
};

#define SE_MALLOC(newNumberOfBytes) GetSubsystem<Memory>.DefaultAlloc()->Reallocate(nullptr, 0, newNumberOfBytes, 1)
#define SE_MALLOC_TYPED(type, newNumberOfElements) reinterpret_cast<type*>(GetSubsystem<Memory>.DefaultAlloc()->Reallocate(nullptr, 0, sizeof(type) * (newNumberOfElements), 1))
#define SE_FREE(oldPointer) GetSubsystem<Memory>.DefaultAlloc()->Reallocate(oldPointer, 0, 0, 1)

#define SE_NEW(type) new (GetSubsystem<Memory>.DefaultAlloc()->Reallocate(nullptr, 0, sizeof(type), 1)) type
#define SE_DELETE(type, oldPointer) \
		do \
		{ \
			if (nullptr != oldPointer) \
			{ \
				typedef type destructor; \
				static_cast<type*>(oldPointer)->~destructor(); \
				GetSubsystem<Memory>.DefaultAlloc()->Reallocate(oldPointer, 0, 0, 1); \
			} \
		} while (0)


SE_NAMESPACE_END
//=============================================================================