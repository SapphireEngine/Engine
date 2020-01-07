#pragma once

template <typename T>
inline void SafeDelete(T *&resource)
{
	delete resource;
	resource = nullptr;
}

template <typename T>
inline void SafeDeleteArray(T *&resource)
{
	delete[] resource;
	resource = nullptr;
}

template <typename T, size_t count>
constexpr size_t LengthOf(const T(&)[count])
{
	return count;
};