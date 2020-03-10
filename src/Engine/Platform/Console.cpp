#include "stdafx.h"
#include "Console.h"

//=============================================================================
SE_NAMESPACE_BEGIN
//-----------------------------------------------------------------------------
struct pimpl
{
#if SE_PLATFORM_WINDOWS
    FILE *inStream = nullptr;
    FILE *outStream = nullptr;
    FILE *errStream = nullptr;
#endif
};
//-----------------------------------------------------------------------------
Console::Console(bool enable)
    : m_enable(enable)
{
    if ( m_enable )
    {
#if SE_PLATFORM_WINDOWS
        m_pimpl = new pimpl;
        ::AllocConsole();
        freopen_s(&m_pimpl->inStream, "conin$", "r", stdin);
        freopen_s(&m_pimpl->outStream, "conout$", "w", stdout);
        freopen_s(&m_pimpl->errStream, "conout$", "w", stderr);
#endif
    }
}
//-----------------------------------------------------------------------------
Console::~Console()
{
    if ( m_enable )
    {
#if SE_PLATFORM_WINDOWS
        ::FreeConsole();
        SafeDelete(m_pimpl);
#endif
    }
}
//-----------------------------------------------------------------------------
SE_NAMESPACE_END
//=============================================================================