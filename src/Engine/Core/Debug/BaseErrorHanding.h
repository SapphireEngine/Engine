#pragma once

//=============================================================================
SE_NAMESPACE_BEGIN

bool IsCriticalError();
void CriticalError();
void CriticalError(const char *text);
const char* GetCriticalErrorText();

SE_NAMESPACE_END
//=============================================================================