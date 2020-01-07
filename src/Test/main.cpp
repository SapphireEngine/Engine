#include "stdafx.h"
#include "Core/Object/Subsystem.h"

using namespace SE_NAMESPACE;

#pragma comment(lib, "Engine.lib")

class Fu : public Subsystem<Fu>
{
public:
	Fu(int ni) : i(ni) { setValid(true); }
	~Fu() {}

	void Pr()
	{
		i = i + 10;
	}

	int i;
};

int main()
{
	Fu::Create(10);
	GetSubsystem<Fu>().Pr();
	Fu::Destroy();

	return 0;
}