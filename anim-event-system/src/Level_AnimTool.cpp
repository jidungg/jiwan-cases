#include "stdafx.h"
#include "Level_AnimTool.h"
#include "GameInstance.h"
#include "TestModelObject.h"

void CLevel_AnimTool::Update_AnimEventImgui()
{

	if (ImGui::Button("Open AnimEvents File"))
	{
		wstring wstrSelectedPath = OpenFileDialog(TEXT("AnimEvent Files\0 * .animevt")); // 파일 다이얼로그 호출
		if (false == wstrSelectedPath.empty())
		{
			if (FAILED(Load_AnimEvents(wstrSelectedPath)))
			{
				MSG_BOX("Failed to Load AnimEvents");
			}
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Export AnimEvents"))
	{
		wstring wstrSelectedPath = SaveFileDialog(TEXT("AnimEvent Files\0 * .animevt")); // 파일 다이얼로그 호출

		if (false == wstrSelectedPath.empty())
		{
			Export_AnimEvents(wstrSelectedPath);
		}
	}
	ImGui::Separator();

	ImGui::Text("Animation Index : %d", m_iCurrentAnimIndex);
	ImGui::Text("Progress : %f", m_fCurrentProgress);

	if (ImGui::Button("Add AnimEvent"))
	{
		ANIM_EVENT tEvent{};
		tEvent.fProgress = m_fCurrentProgress;
		tEvent.iAnimIndex = m_iCurrentAnimIndex;
		m_AnimEvents[m_iCurrentAnimIndex].push_back(tEvent);
	}
	ImGui::SameLine();
	if (ImGui::Button("Remove AnimEvent"))
	{
		if (m_iSelectedAnimEventIndex != -1)
		{
			_int iTmpIndex = m_iSelectedAnimEventIndex;
			for (auto& pairAnimEvt : m_AnimEvents)
			{
				if ((iTmpIndex - (_int)pairAnimEvt.second.size()) < 0)
				{
					pairAnimEvt.second.erase(pairAnimEvt.second.begin() + iTmpIndex);
					break;
				}
				iTmpIndex -= (_int)pairAnimEvt.second.size();
			}
		}
	}
	ImGui::Separator();

	_uint iEventIdx = 0;
	if (ImGui::BeginListBox("AnimEventList"))
	{
		for (auto& pairAnimEvt : m_AnimEvents)
		{
			for (auto& tEvent : pairAnimEvt.second)
			{
				_bool isSelected = iEventIdx == m_iSelectedAnimEventIndex;
				string strLable = to_string(iEventIdx) + " AnimIdx : " + to_string(pairAnimEvt.first) + "/ Progress : " + to_string(tEvent.fProgress);
				if (ImGui::Selectable(strLable.c_str(), isSelected))
				{
					m_iSelectedAnimEventIndex = iEventIdx; // 선택된 항목 갱신
				}
				// 선택된 항목 강조
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
				iEventIdx++;
			}
		}
		ImGui::EndListBox();
	}
	ImGui::Separator();
	if (m_iSelectedAnimEventIndex != -1)
	{
		ANIM_EVENT* tEvent = nullptr;
		_int iTmpIdx = m_iSelectedAnimEventIndex;
		for (auto& pairAnimEvt : m_AnimEvents)
		{
			if ((iTmpIdx - (_int)pairAnimEvt.second.size()) < 0)
			{
				tEvent = &pairAnimEvt.second[iTmpIdx];
				break;
			}
			iTmpIdx -= (_int)pairAnimEvt.second.size();
		}
		if (tEvent)
		{
			ImGui::Text("Selected Animation Event Data");
			_int iAnimIndex = (_uint)tEvent->iAnimIndex;
			if (ImGui::InputInt("Animation Index", &iAnimIndex))
			{
				_int iClampedAnimIndex = iAnimIndex;
				iClampedAnimIndex = max(0, iAnimIndex);
				if (m_pTestModelObj)
					iClampedAnimIndex = min(iAnimIndex, (_int)m_pTestModelObj->Get_AnimationCount() - 1);
				m_AnimEvents[iClampedAnimIndex].push_back(*tEvent);
				m_AnimEvents[(_uint)tEvent->iAnimIndex].erase(m_AnimEvents[(_uint)tEvent->iAnimIndex].begin() + iTmpIdx);
				tEvent = &m_AnimEvents[iClampedAnimIndex].back();
				tEvent->iAnimIndex = iClampedAnimIndex;
			}
			_float fProgress = tEvent->fProgress;
			if (ImGui::InputFloat("Progress", &fProgress))
			{
				fProgress = max(0, fProgress);
				fProgress = fmin(fProgress, 1.f);
				tEvent->fProgress = fProgress;
			}

			char buffer[m_iFuncNameBufferSize] = "";
			strcpy_s(buffer, tEvent->strFuncName.c_str());
			if (ImGui::InputText("FunctionName", buffer, m_iFuncNameBufferSize)) {
				tEvent->strFuncName = buffer;
			}
		}

	}
}

HRESULT CLevel_AnimTool::Load_AnimEvents(wstring _wstrPath)
{
	assert(!_wstrPath.empty());

	SetWindowText(g_hWnd, TEXT("모델(을)를 로딩중입니다."));

	std::ifstream inFile(_wstrPath, std::ios::binary);
	if (!inFile) {
		string str = "파일을 열 수 없습니다.";
		str += WstringToString(_wstrPath);
		MessageBoxA(NULL, str.c_str(), "에러", MB_OK);
		inFile.close();
		return E_FAIL;
	}
	m_AnimEvents.clear();
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

HRESULT CLevel_AnimTool::Export_AnimEvents(const wstring& _wstrPath)
{
	if (m_AnimEvents.empty())
	{
		MSG_BOX("이벤트가 없는데요?");
		return E_FAIL;
	}
	wstring wstrExt = TEXT(".animevt");
	std::filesystem::path path = _wstrPath;
	if (path.extension().wstring() != wstrExt)
	{
		path.replace_extension(wstrExt);
	}
	std::ofstream outFile(path, std::ios::binary);
	if (!outFile)
	{
		outFile.close();
		MSG_BOX("파일 열기 실패.");
		return E_FAIL;
	}
	_uint iAnimIndexCount = (_uint)m_AnimEvents.size();
	outFile.write(reinterpret_cast<char*>(&iAnimIndexCount), sizeof(_uint));
	for (auto& pairAnimEvt : m_AnimEvents)
	{
		_uint iAnimIndex = pairAnimEvt.first;
		_uint iEventCount = (_uint)pairAnimEvt.second.size();
		outFile.write(reinterpret_cast<char*>(&iAnimIndex), sizeof(_uint));
		outFile.write(reinterpret_cast<char*>(&iEventCount), sizeof(_uint));
		for (auto& tEvent : pairAnimEvt.second)
		{
			tEvent.WriteFile(outFile);
		}
	}
	outFile.close();
	return S_OK;
}
