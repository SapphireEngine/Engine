#include "stdafx.h"
#include "Platform\Console\Console.h"

//=============================================================================
SE_NAMESPACE_BEGIN
//-----------------------------------------------------------------------------
struct pimpl
{
    FILE *inStream = nullptr;
    FILE *outStream = nullptr;
    FILE *errStream = nullptr;
};
//-----------------------------------------------------------------------------
Console::Console(bool enable)
    : m_enable(enable)
{
    if ( m_enable )
    {
        m_pimpl = new pimpl;
        AllocConsole();
        freopen_s(&m_pimpl->inStream, "conin$", "r", stdin);
        freopen_s(&m_pimpl->outStream, "conout$", "w", stdout);
        freopen_s(&m_pimpl->errStream, "conout$", "w", stderr);
    }
}
//-----------------------------------------------------------------------------
Console::~Console()
{
    if ( m_enable )
    {
        FreeConsole();
        SafeDelete(m_pimpl);
    }
}
//-----------------------------------------------------------------------------
SE_NAMESPACE_END
//=============================================================================