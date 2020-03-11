#pragma once

//=============================================================================
SE_NAMESPACE_BEGIN

namespace stl
{
#if SE_EASTL_ENABLED
	using namespace eastl;
#else
	using namespace std;
#endif
}

SE_NAMESPACE_END
//=============================================================================