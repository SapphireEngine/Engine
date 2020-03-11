#pragma once

//=============================================================================
SE_NAMESPACE_BEGIN

typedef enum FileMode
{
    FM_READ = 1 << 0,
    FM_WRITE = 1 << 1,
    FM_APPEND = 1 << 2,
    FM_BINARY = 1 << 3,
    FM_READ_WRITE = FM_READ | FM_WRITE,
    FM_READ_APPEND = FM_READ | FM_APPEND,
    FM_WRITE_BINARY = FM_WRITE | FM_BINARY,
    FM_READ_BINARY = FM_READ | FM_BINARY,
    FM_APPEND_BINARY = FM_APPEND | FM_BINARY,
    FM_READ_WRITE_BINARY = FM_READ | FM_WRITE | FM_BINARY,
    FM_READ_APPEND_BINARY = FM_READ | FM_APPEND | FM_BINARY,
} FileMode;

SE_NAMESPACE_END
//=============================================================================