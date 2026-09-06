# 문제 해결 사례 모음

김지완 · [KJW92525766@gmail.com](mailto:KJW92525766@gmail.com) · [github.com/jidungg](https://github.com/jidungg)

게임 클라이언트·엔진 프로그래밍에서 마주친 문제와 그 해결 과정을 사례 단위로
정리한 문서 9건입니다. 각 사례는 하나의 문제만 다루고, 배경 → 구현 →
장단점 → 대안 비교 순서로 씁니다. 쓸 말이 있으면 회고를 덧붙입니다.

**장단점 섹션의 단점은 비워두지 않습니다.** 수치는 대안 비교 섹션에 측정
조건과 함께 적습니다.

기계로 생성한 표 형태의 색인은 [INDEX.md](INDEX.md)에 있습니다.

## Dogong — Untitled-Forge

5인 팀 프로젝트, 클라이언트 프로그래머. 2026-04 ~ 2026-08. 언리얼 엔진.

- [익스트랙션 진입·추출의 서버·레벨 구조](single-world-region-teleport/single-world-region-teleport.md)
  — 개인 탈출과 리슨 서버 구조가 충돌해 배치안 3개를 비교했습니다. 기지와 약탈
  지역을 한 월드에 담아 travel을 없애고 서버 권위 텔레포트로 통일했습니다.
  (서버·레벨 구조 설계 담당)
- [팀 공용 AI 빌드 검증 스킬 제작 경험](build-verification-skill/build-verification-skill.md)
  — 커밋 전 빌드 통과라는 팀 규칙을 AI 에이전트가 자동으로 지키게 만든 스킬.
  엔진 경로 해석을 4단계로 스킬 안에 옮겨 팀원이 무설정으로 쓰게 했습니다.
  (Git 워크플로우 문서·브랜치 작업 스킬·빌드 검증 스킬 단독 작성)
- [AI 메모리 뱅크 구축과 실패 경험](ai-memory-bank/ai-memory-bank.md)
  — 프로젝트의 결정과 이유를 AI가 기억하게 하려고 만든 5개 파일(733줄)과 관리
  커맨드 3개. 추적이 어려워 폐기했고 팀원의 더 작은 devlog 시스템이 그 자리를
  대체했습니다. (메모리 뱅크 구축·운영과 관리 커맨드 단독 담당)

## RTChess — Hyper Chess 모작

1인 개발, 클라이언트 프로그래머. 2026-01 ~ 2026-03. 언리얼 엔진.

- [실시간 체스 기력 동기화](energy-client-prediction/energy-client-prediction.md)
  — UI 표시용 기력 값의 복제 빈도를 초당 100회에서 1회로 낮췄습니다. 이후 드러난
  지연 환경의 UI 반응 지연과 표시값이 튀는 현상을 로컬 소비 예측과 서버 승인
  시퀀스로 차례로 잡았습니다.

## CopyMaple2 — 메이플스토리2 모작

1인 개발, 클라이언트/엔진 프로그래머. 2024-10 ~ 2024-12, 2026-02. DirectX 11.

- [그리드 큐브맵 Raycast에 3D DDA 도입](raycast-3d-dda/raycast-3d-dda.md)
  — 전체 셀 순회 대비 평균 실행 시간을 14.9μs에서 0.2μs로 줄였습니다.
- [하우징(건축) 시스템](housing-map-serialization/housing-map-serialization.md)
  — 그리드 큐브맵 위에 블록을 배치하는 UGC 하우징을 만들고 그 건설 모드를 게임 맵
  제작 툴로도 썼습니다. 연속 블록 압축으로 맵 저장 객체를 65% 줄였습니다.
- [nif → .effmodel 익스포터 심기](nif-to-effmodel/nif-to-effmodel.md)
  — 포맷 명세를 구할 수 없어, nif를 읽는 오픈소스 툴 NifSkope를 분석해 그 파싱
  결과를 자체 포맷으로 써내는 익스포터를 툴 안에 심었습니다. 이펙트 102개를
  게임에 올렸습니다.

## Project_DX11_PluckySquire — 견습 기사 모험기 모작

7인 팀 프로젝트, 엔진/클라이언트 프로그래머(애니메이션·플레이어 담당).
2025-01 ~ 2025-03. DirectX 11.

- [애니메이션 이벤트 시스템](anim-event-system/anim-event-system.md)
  — 애니메이션 진행도에 이벤트를 찍어 별도 파일로 저장하고, 런타임에 문자열 키로
  콜백을 호출하는 시스템.
- [플레이어 상태 머신](player-state-machine/player-state-machine.md)
  — 상태를 클래스 단위로 분리해 관리한 구조. 확장은 쉬웠지만 상태 클래스가 36개까지
  늘면서 생긴 네 가지 문제를 개발 중에 문서로 정리했습니다.

## 팀 프로젝트 기여 범위

Dogong과 Project_DX11_PluckySquire는 팀 프로젝트입니다. 위 목록과 각 사례 본문에
담당한 부분을 표시했고, 팀 성과를 개인 성과로 합치지 않았습니다.

## 이 저장소에 대해

이 저장소는 [jiwan-portfolio](https://github.com/jidungg/jiwan-portfolio)의
`Cases/` 디렉토리를 `git subtree split`으로 내보낸 것입니다. 사례 본문은 원본
저장소에서 관리하므로, 수정이 필요하면 여기가 아니라 원본에 반영해야 합니다.
