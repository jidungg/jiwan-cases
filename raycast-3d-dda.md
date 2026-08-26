---
id: raycast-3d-dda
title: Raycast 98.7% 단축 — 자체 구현 대신 표준 3D DDA 알고리즘 도입
status: draft
summary: 그리드 기반 큐브맵에서 2축(X/Z)만 처리하던 자체 Raycast 구현을 표준 3D DDA로 교체해 평균 실행 시간을 14.9μs에서 0.2μs로 줄임 (98.7% 감소)
project: CopyMaple2 (메이플스토리2 모작)
role: 클라이언트/엔진 프로그래머 (개인 프로젝트, 1인 개발)
period: 2026-02 (정확한 소요 기간은 기억나지 않음 — 리팩토링 커밋 1건으로 반영됨)
engine: [자체엔진 (DirectX11)]
lang: [C++]
domain: [최적화]
skills: [알고리즘, 프로파일링, 충돌 검사, 자료구조]
sources:
  - "Sources/Projects.md의 CopyMaple2 항목 — 로컬: D:\\Workbench\\Projects\\CopyMaple2\\Client\\Private\\Terrain.cpp (RayCast 함수 276-357행, NUM0 마이크로벤치 68-84행)"
  - https://github.com/jidungg/CopyMaple2/commit/c4abf02c6ed3af3647d0d25624dec8fea704efe1
  - "Sources/Portfolios.md — Notion '최적화 사례 (3D 큐브 맵 RPG, MapleStory2 모작)' 2.2절"
confidence: high
pages: 2
---

## 상황

CopyMaple2(메이플스토리2 모작, DirectX11 자체 엔진, 개인 프로젝트)는 그리드 기반
큐브맵으로 지형을 구성하고 있습니다. 캐릭터 충돌 처리를 프로파일링해봤더니, 충돌
검사/처리가 게임 로직 파트에서 가장 높은 CPU 점유율(12.98%)을 차지하고 있었고,
그중에서도 Raycast 비용이 절반 이상을 차지하고 있었습니다. Raycast는 캐릭터 이동
시 지형 블로킹 판정, 스킬 판정 등 여러 곳에서 매 프레임 반복 호출되는 핵심
경로였습니다.

![플레이어 캐릭터의 충돌 검사 관련 CPU 사용량 측정 결과](../Assets/raycast-3d-dda_cpu-usage.png)

## 제약

그리드 맵이라 임의의 3D 좌표를 셀 인덱스로 바로 변환할 수는 있었지만, Ray가
시작점에서 목표까지 **어떤 순서로 셀을 통과하는지**는 따로 추적해야 했습니다. 이
순서를 모르면 가까운 셀에서 충돌이 발견돼도 먼 셀까지 계속 검사하게 되거나,
반대로 검사 순서가 꼬여 잘못된 셀에서 먼저 충돌 판정이 나버리는 문제가
생깁니다.

## 접근

- **기존 구현: `RayCastXZ`** — X/Z 두 축만 다루던 자체 구현이었습니다(함수 이름
  그대로입니다). 0으로 나누는 걸 막으려고 `(fXRequire + 1e-6f) / (fXDir +
  1e-6f)` 같은 임시방편을 썼고, 매 스텝마다 X축 도달 시간과 Z축 도달 시간만
  비교해서 다음 위치로 이동했습니다. Y축은 아예 고려하지 않아서, 3차원
  큐브맵에서는 통과 순서를 정확히 보장할 수 없었습니다. 결국 2026-02-10
  커밋에서 이 함수를 삭제했습니다.
- **선택한 안: 표준 3D DDA(Voxel Traversal)** — 기술 리서치로 찾아낸
  알고리즘입니다. X/Y/Z 세 축 모두에 대해 "셀 한 칸을 이동하는 데 걸리는
  비용(TDelta)"과 "다음 경계면까지의 거리(TMax)"를 미리 계산해두고, 매 스텝마다
  TMax가 가장 작은 축으로만 한 칸씩 전진합니다. 축 개수에 상관없이 일반화되는
  알고리즘이라 Y축도 자연스럽게 포함시킬 수 있었고, 임시방편 코드도
  사라졌습니다.

![3D DDA 알고리즘 개념도](../Assets/raycast-3d-dda_algorithm-diagram.png)

## 구현

`RayCast` 함수의 DDA 전진 로직입니다. 세 축 중 가장 먼저 경계에 도달하는 축을
골라 그쪽으로만 한 칸 전진하고, 그 축의 TMax를 다음 경계까지 갱신합니다.

```cpp
// DDA step
if (fTMaxX < fTMaxY && fTMaxX < fTMaxZ)
{
    iX += iStepX;
    fT = fTMaxX;
    fTMaxX += fTDeltaX;
}
else if (fTMaxZ < fTMaxY)
{
    iZ += fStepZ;
    fT = fTMaxZ;
    fTMaxZ += fTDeltaZ;
}
else
{
    iY += iStepY;
    fT = fTMaxY;
    fTMaxY += fTDeltaY;
}
```

이 루프가 한 바퀴 돌 때마다 Ray가 통과하는 다음 셀 하나를 정확한 순서로 얻고,
그 셀에서만 실제 충돌 검사(`m_vecCubes[iIdx]->RayCast(...)`)를 수행합니다.

## 결과

동일 씬, Release x64, 10만 회 평균 기준입니다.

- 3D DDA 미적용(전체 셀 순회): 평균 14.9μs / 회
- 3D DDA 적용: 평균 0.2μs / 회
- **약 98.7% 실행 시간 감소**

## 회고

`RayCastXZ`처럼 임시방편(2축 한정, epsilon 처리)을 먼저 만들고 나서야 한계에
부딪혀 표준 알고리즘을 찾아본 순서가 아쉽습니다. 비슷한 문제(격자에서 방향 있는
순회)를 만나면 직접 궁리하기 전에 이미 알려진 표준 해법이 있는지부터 찾아보는
습관을 이 경험 이후로 들이게 됐습니다.
