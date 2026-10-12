#include "AnimEventGenerator.h"
#include "Model.h"
#include "AnimEventReceiver.h"

HRESULT CAnimEventGenerator::Initialize_Prototype(const _char* _pAnimEventFilePath)
{
	std::ifstream inFile(_pAnimEventFilePath, std::ios::binary);
	if (!inFile) {
		string str = "파일을 열 수 없습니다.";
		str += _pAnimEventFilePath;
		MessageBoxA(NULL, str.c_str(), "에러", MB_OK);
		return E_FAIL;
	}
	_uint iAnimIndexCount = 0;
	inFile.read(reinterpret_cast<char*>(&iAnimIndexCount), sizeof(_uint));
	for (_uint i = 0; i < iAnimIndexCount; i++)
	{
		_uint iAnimIndex = 0;
		_uint iEventCount = 0;
		inFile.read(reinterpret_cast<char*>(&iAnimIndex), sizeof(_uint));
		inFile.read(reinterpret_cast<char*>(&iEventCount), sizeof(_uint));
		m_AnimEvents.emplace(iAnimIndex, vector<ANIM_EVENT>());
		vector<ANIM_EVENT>& Events = m_AnimEvents[iAnimIndex];
		Events.resize(iEventCount);
		for (_uint j = 0; j < iEventCount; j++)
		{
			Events[j].ReadFile(inFile);
		}
	}

	inFile.close();
	return S_OK;
}

HRESULT CAnimEventGenerator::Initialize(void* _pArg)
{
	ANIMEVTGENERATOR_DESC* pDesc = (ANIMEVTGENERATOR_DESC*)_pArg;
	m_pSenderModel = pDesc->pSenderModel;
	m_pReceiver = pDesc->pReceiver;
	m_pSenderModel->Set_AnimEventGenerator(this);
	__super::Initialize(_pArg);
	return S_OK;
}

void CAnimEventGenerator::Update(_float _fTimeDelta)
{
	_uint iCurAnimIdx = m_pSenderModel->Get_CurrentAnimIndex();
	if (m_AnimEvents.find(iCurAnimIdx) == m_AnimEvents.end())
		return;
	vector<ANIM_EVENT>& Events = m_AnimEvents[iCurAnimIdx];
	_float fProgress = m_pSenderModel->Get_CurrentAnimProgeress();
	for (ANIM_EVENT& tEvnt : Events)
	{
		if (tEvnt.bIsTriggered || false == m_pSenderModel->Is_DuringAnimation()) continue;

		_bool bReached = m_pSenderModel->Is_ReversingAnimation()
			? (fProgress <= tEvnt.fProgress) : (fProgress >= tEvnt.fProgress);

		if (bReached)
		{
			m_pReceiver->Invoke(tEvnt.strFuncName);
			tEvnt.bIsTriggered = true;
		}
	}
}

#ifdef _TOOL
HRESULT CAnimEventGenerator::Export_AnimEvents(ofstream& _OutFile)
{
	if (m_AnimEvents.empty())
	{
		MSG_BOX("이벤트가 없는데요?");
		return E_FAIL;
	}

	_uint iAnimIndexCount = m_AnimEvents.size();
	_OutFile.write(reinterpret_cast<char*>(&iAnimIndexCount), sizeof(_uint));
	for (auto& pairAnimEvt : m_AnimEvents)
	{
		_uint iAnimIndex = pairAnimEvt.first;
		_uint iEventCount = pairAnimEvt.second.size();
		_OutFile.write(reinterpret_cast<char*>(&iAnimIndex), sizeof(_uint));
		_OutFile.write(reinterpret_cast<char*>(&iEventCount), sizeof(_uint));
		for (auto& tEvent : pairAnimEvt.second)
		{
			tEvent.WriteFile(_OutFile);
		}
	}
}
#endif // _TOOL