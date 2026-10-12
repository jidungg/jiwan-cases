#include "Animation.h"

void CAnimation::Reset(_bool _bReverse)
{
	Set_Progress(_bReverse ? 1.f : 0.f, _bReverse);
	for (auto& pAnimEvent : m_AnimEvents)
	{
		pAnimEvent->bIsTriggered = false;
	}
}

