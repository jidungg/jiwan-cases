#pragma once
BEGIN(Engine)
class IAnimEventReceiver
{
public:
	void Bind_AnimEventFunc(string _strFuncName, function<void()> _Func)
	{
		m_mapFunc[_strFuncName] = _Func;
	}
	void Invoke(string _strFuncName)
	{
		auto iter = m_mapFunc.find(_strFuncName);
		if (iter == m_mapFunc.end())
			return;
		iter->second();
	}
protected:
	map<string, function<void()>> m_mapFunc;
};

END