#include "stdafx.h"
#include "BaseErrorHanding.h"

//=============================================================================
SE_NAMESPACE_BEGIN
//-----------------------------------------------------------------------------
static bool isCriticalError = false;
const char *criticalErrorText = "";
//-----------------------------------------------------------------------------
bool IsCriticalError()
{
	return isCriticalError;
}
//-----------------------------------------------------------------------------
void CriticalError()
{
	isCriticalError = true;
}
//-----------------------------------------------------------------------------
void CriticalError(const char *text)
{
	criticalErrorText = text;
	CriticalError();
}
//-----------------------------------------------------------------------------
const char* GetCriticalErrorText()
{
	return criticalErrorText;
}
//-----------------------------------------------------------------------------
SE_NAMESPACE_END
//=============================================================================