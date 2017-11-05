#pragma once
#include "..\Enums\UidSize.h"

struct RfidCard
{
	UidSize length;
	uint8_t cardUid[7] = { 0, 0, 0, 0, 0, 0, 0 };
};
