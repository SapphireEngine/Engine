#pragma once

//=============================================================================
SE_NAMESPACE_BEGIN

class Console
{
public:
	Console(bool enable);
	~Console();

private:
	struct pimpl *m_pimpl = nullptr;
	bool m_enable;
};

SE_NAMESPACE_END
//=============================================================================