#pragma once
#include "Component.h"
BEGIN(Engine)
class IAnimEventReceiver;
typedef struct tagAnimEvent
{
	void WriteFile(std::ofstream& _outFile)
	{
		_outFile.write(reinterpret_cast<char*>(&iAnimIndex), sizeof(_uint));
		_uint iLength = (_uint)strFuncName.length();
		_outFile.write(reinterpret_cast<char*>(&iLength), sizeof(_uint));
		_outFile.write(strFuncName.c_str(), iLength);
		_outFile.write(reinterpret_cast<char*>(&fProgress), sizeof(_float));
	}
	void ReadFile(std::ifstream& _inFile)
	{
		_inFile.read(reinterpret_cast<char*>(&iAnimIndex), sizeof(_uint));
		_uint iLength = 0;
		_inFile.read(reinterpret_cast<char*>(&iLength), sizeof(_uint));
		strFuncName.resize(iLength);
		_inFile.read(const_cast<char*>(strFuncName.c_str()), iLength);
		_inFile.read(reinterpret_cast<char*>(&fProgress), sizeof(_float));
	}
	_uint iAnimIndex = 0;
	string strFuncName;
	_float fProgress;
	_bool bIsTriggered = false;
}ANIM_EVENT;
class CModel;
class ENGINE_DLL CAnimEventGenerator :
	public CComponent
{
public:
	typedef struct tagAnimEventGeneratorDesc
	{
		CModel* pSenderModel = nullptr;
		IAnimEventReceiver* pReceiver = nullptr;
	}ANIMEVTGENERATOR_DESC;
public:
	virtual HRESULT Initialize_Prototype(const _char* _pAnimEventFilePath);
	virtual HRESULT Initialize(void* _pArg) override;
	virtual void Update(_float _fTimeDelta) override;

	map<_uint, vector<ANIM_EVENT>>& Get_AnimEvents() { return m_AnimEvents; }
#ifdef _TOOL
public:
	HRESULT Export_AnimEvents(ofstream& _OutFile);
#endif // _TOOL

private:
	CModel* m_pSenderModel = nullptr;
	IAnimEventReceiver* m_pReceiver = nullptr;
	//Key : AnimIndex, Value : AnimEventVector
	map<_uint, vector<ANIM_EVENT>> m_AnimEvents;
};

END