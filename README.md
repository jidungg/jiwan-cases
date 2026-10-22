# 문제 해결 사례 모음

김지완 · [KJW92525766@gmail.com](mailto:KJW92525766@gmail.com) · [github.com/jidungg](https://github.com/jidungg)

게임 클라이언트·엔진 프로그래밍에서 마주친 문제와 그 해결 과정을 사례 단위로
정리했습니다. 각 사례는 하나의 문제만 다룹니다.

## 이 저장소에 있는 것

* **폴더 하나가 사례 하나입니다.** `<사례명>/<사례명>.md`가 본문이고, 본문에 쓰인
이미지가 같은 폴더에 있습니다. 코드가 있는 사례는 `src/`에 핵심 .h/.cpp 사본을
두고 본문 끝 '첨부 코드'에서 연결합니다. 
* 사례 본문은 배경 → 구현 → 장단점 → 대안 비교 → 회고 → 첨부 코드 순서입니다.
회고와 첨부 코드는 없는 사례도 있습니다.
* [INDEX.md](INDEX.md)는 전체 사례를 id·프로젝트·분야·기술 열로 정리한 표입니다.

## 관심 분야별로 보기

|분야|사례|
|-|-|
|최적화|[그리드 큐브맵 Raycast에 3D DDA 도입](raycast-3d-dda/raycast-3d-dda.md)|
|네트워크|[실시간 체스 기력 동기화](energy-client-prediction/energy-client-prediction.md), [익스트랙션 진입·추출의 서버·레벨 구조](single-world-region-teleport/single-world-region-teleport.md)|
|애니메이션|[스켈레톤이 다른 기물 9종의 애님 그래프 공유](anim-layer-sharing/anim-layer-sharing.md), [자체 엔진 2D·3D 애니메이션 시스템](skeletal-animation-system/skeletal-animation-system.md)|
|설계|[익스트랙션 진입·추출의 서버·레벨 구조](single-world-region-teleport/single-world-region-teleport.md), [스켈레톤이 다른 기물 9종의 애님 그래프 공유](anim-layer-sharing/anim-layer-sharing.md), [플레이어 상태 머신](player-state-machine/player-state-machine.md), [자체 엔진 2D·3D 애니메이션 시스템](skeletal-animation-system/skeletal-animation-system.md), [애니메이션 이벤트 시스템](anim-event-system/anim-event-system.md)|
|게임플레이|[실시간 체스 기력 동기화](energy-client-prediction/energy-client-prediction.md), [하우징(건축) 시스템](housing-map-serialization/housing-map-serialization.md), [플레이어 상태 머신](player-state-machine/player-state-machine.md)|
|툴·에셋 파이프라인|[애니메이션 이벤트 시스템](anim-event-system/anim-event-system.md), [팀 공용 AI 빌드 검증 스킬 제작 경험](build-verification-skill/build-verification-skill.md), [하우징(건축) 시스템](housing-map-serialization/housing-map-serialization.md), [nif → .effmodel 익스포터 심기](nif-to-effmodel/nif-to-effmodel.md)|
|프로세스·협업|[AI 메모리 뱅크 구축과 실패 경험](ai-memory-bank/ai-memory-bank.md), [팀 공용 AI 빌드 검증 스킬 제작 경험](build-verification-skill/build-verification-skill.md)|

한 사례가 여러 분야에 걸치면 각 줄에 모두 넣었습니다.

## 프로젝트 소개

사례는 아래 네 프로젝트에서 나왔습니다.

|프로젝트|어떤 게임|형태|기간|엔진|역할·담당|
|-|-|-|-|-|-|
|Dogong|Untitled-Forge|5인 팀|2026-04 \~ 2026-08|언리얼 엔진|클라이언트 프로그래머, 서버·레벨 구조 설계|
|RTChess|Hyper Chess 모작|1인 개발|2026-01 \~ 2026-03|언리얼 엔진|클라이언트 프로그래머|
|CopyMaple2|메이플스토리2 모작|1인 개발|2024-10 \~ 2024-12, 2026-02|DirectX 11 자체 엔진|클라이언트/엔진 프로그래머|
|Project\_DX11\_PluckySquire|견습 기사 모험기 모작|7인 팀|2025-01 \~ 2025-03|DirectX 11 자체 엔진|엔진/클라이언트 프로그래머 (애니메이션·플레이어 담당)|

Dogong과 Project\_DX11\_PluckySquire는 팀 프로젝트입니다. 아래 목록과 각 사례 본문에
담당한 부분을 표시했고, 팀 성과를 개인 성과로 합치지 않았습니다.

## 프로젝트별 사례 목록

### Dogong

* [익스트랙션 진입·추출의 서버·레벨 구조](single-world-region-teleport/single-world-region-teleport.md)
— 개인 탈출과 리슨 서버 구조가 충돌해 배치안 3개를 비교했습니다. 기지와 약탈
지역을 한 월드에 담아 travel을 없애고 서버 권위 텔레포트로 통일했습니다.
(서버·레벨 구조 설계 담당)
* [팀 공용 AI 빌드 검증 스킬 제작 경험](build-verification-skill/build-verification-skill.md)
— 커밋 전 빌드 통과라는 팀 규칙을 AI 에이전트가 자동으로 지키게 만든 스킬.
엔진 경로 해석을 4단계로 스킬 안에 옮겨 팀원이 무설정으로 쓰게 했습니다.
(Git 워크플로우 문서·브랜치 작업 스킬·빌드 검증 스킬 단독 작성)
* [AI 메모리 뱅크 구축과 실패 경험](ai-memory-bank/ai-memory-bank.md)
— 프로젝트의 결정과 이유를 AI가 기억하게 하려고 만든 5개 파일(733줄)과 관리
커맨드 3개. 추적이 어려워 폐기했고 팀원의 더 작은 devlog 시스템이 그 자리를
대체했습니다. (메모리 뱅크 구축·운영과 관리 커맨드 단독 담당)

### RTChess — Hyper Chess 모작

* [실시간 체스 기력 동기화](energy-client-prediction/energy-client-prediction.md)
— UI 표시용 기력 값의 복제 빈도를 초당 100회에서 1회로 낮췄습니다. 이후 드러난
지연 환경의 UI 반응 지연과 표시값이 튀는 현상을 로컬 소비 예측과 서버 승인
시퀀스로 차례로 잡았습니다.
* [스켈레톤이 다른 기물 9종의 애님 그래프 공유](anim-layer-sharing/anim-layer-sharing.md)
— 템플릿 애니메이션 블루프린트 하나에 상태 머신을 두고, 스켈레톤별 블루프린트는
시퀀스 변수만 채우며, 나이트 점프·폰 공격은 링크드 애님 레이어로 갈아 끼웠습니다.

### CopyMaple2 — 메이플스토리2 모작

* [그리드 큐브맵 Raycast에 3D DDA 도입](raycast-3d-dda/raycast-3d-dda.md)
— 전체 셀 순회 대비 평균 실행 시간을 2.74μs에서 0.06μs로 줄였습니다.
* [하우징(건축) 시스템](housing-map-serialization/housing-map-serialization.md)
— 그리드 큐브맵 위에 블록을 배치하는 UGC 하우징을 만들고 그 건설 모드를 게임 맵
제작 툴로도 썼습니다. 연속 블록 압축으로 맵 저장 객체를 65% 줄였습니다.
* [nif → .effmodel 익스포터 심기](nif-to-effmodel/nif-to-effmodel.md)
— 포맷 명세를 구할 수 없어, nif를 읽는 오픈소스 툴 NifSkope를 분석해 그 파싱
결과를 자체 포맷으로 써내는 익스포터를 툴 안에 심었습니다. 이펙트 102개를
게임에 올렸습니다.

### Project\_DX11\_PluckySquire — 견습 기사 모험기 모작

* [자체 엔진 2D·3D 애니메이션 시스템](skeletal-animation-system/skeletal-animation-system.md)
— 키프레임 보간·전환 보간·뼈 행렬 합성과 스프라이트 서브프레임 재생을 만들고,
FBX·Paper2D 원본을 엔진 포맷으로 변환하는 경로를 붙였습니다. (기본 3D 재생은
학원 템플릿 참고, 나머지 단독 구현)
* [애니메이션 이벤트 시스템](anim-event-system/anim-event-system.md)
— 애니메이션 진행도에 이벤트를 찍어 별도 파일로 저장하고, 런타임에 문자열 키로
콜백을 호출하는 시스템.
* [플레이어 상태 머신](player-state-machine/player-state-machine.md)
— 상태를 클래스 단위로 분리해 관리한 구조. 확장은 쉬웠지만 상태 클래스가 36개까지
늘면서 생긴 네 가지 문제를 개발 중에 문서로 정리했습니다.

