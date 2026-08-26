---
id: raycast-3d-dda
title: 그리드 큐브맵 Raycast에 3D DDA 도입 — 전체 순회 대비 98.7% 단축
summary: 그리드 기반 큐브맵의 Raycast에 3D DDA(Voxel Traversal)를 도입해, 전체 셀 순회 대비 평균 실행 시간을 14.9μs에서 0.2μs로 줄임 (98.7% 감소)
project: CopyMaple2 (메이플스토리2 모작)
role: 클라이언트/엔진 프로그래머 (개인 프로젝트, 1인 개발)
period: 2026-02
engine: [자체엔진 (DirectX11)]
lang: [C++]
domain: [최적화]
skills: [알고리즘, 프로파일링, 충돌 검사, 자료구조]
sources:
  - Sources/Projects.md — CopyMaple2 (Client/Private/Terrain.cpp, CTerrain::RayCast)
  - https://github.com/jidungg/CopyMaple2/commit/c4abf02c6ed3af3647d0d25624dec8fea704efe1
  - "Sources/Portfolios.md — Notion '최적화 사례 (3D 큐브 맵 RPG, MapleStory2 모작)' 2.2절"
pages: 2
---

## 배경

CopyMaple2(메이플스토리2 모작, DirectX11 자체 엔진, 개인 프로젝트)는 그리드 기반
큐브맵으로 지형을 구성하고 있습니다. 캐릭터 충돌 처리를 프로파일링해봤더니, 충돌
검사/처리가 게임 로직 파트에서 가장 높은 CPU 점유율(12.98%)을 차지하고 있었고,
그중에서도 Raycast 비용이 절반 이상을 차지하고 있었습니다. Raycast는 캐릭터 이동
시 지형 블로킹 판정, 스킬 판정 등 여러 곳에서 매 프레임 반복 호출되는 핵심
경로였습니다.

![플레이어 캐릭터의 충돌 검사 관련 CPU 사용량 측정 결과](../Assets/raycast-3d-dda_cpu-usage.png)

문제를 어렵게 만든 조건은 이렇습니다. 그리드 맵이라 임의의 3D 좌표를 셀 인덱스로
바로 변환할 수는 있었지만, Ray가 시작점에서 목표까지 **어떤 순서로 셀을
통과하는지**는 따로 추적해야 했습니다. 이 순서를 모르면 가까운 셀에서 충돌이
발견돼도 먼 셀까지 계속 검사하게 되거나, 반대로 검사 순서가 꼬여 잘못된 셀에서
먼저 충돌 판정이 나버립니다.

## 구현

3D DDA(Voxel Traversal)를 적용했습니다. X/Y/Z 세 축 각각에 대해 "셀 한 칸을
이동하는 데 드는 비용(TDelta)"과 "다음 경계면까지 남은 거리(TMax)"를 미리
계산해두고, 매 스텝마다 TMax가 가장 작은 축으로만 한 칸씩 전진하는 방식입니다.

![3D DDA 알고리즘 개념도](../Assets/raycast-3d-dda_algorithm-diagram.png)

`CTerrain::RayCast`의 전진 로직입니다. 세 축 중 가장 먼저 경계에 닿는 축을 골라
그쪽으로 한 칸 가고, 그 축의 TMax를 다음 경계까지 갱신합니다.

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

## 장단점

- **장점** — 검사 비용이 맵 크기와 완전히 무관해집니다. Ray가 실제로 지나는
  셀만 보므로 비용이 Ray 길이에만 비례하고, 맵이 아무리 넓어져도 그대로입니다.
  셀을 가까운 순서대로 방문하기 때문에 첫 충돌을 만나면 바로 반환할 수 있다는
  점도 이득입니다.

- **단점** — 균일 그리드를 전제로 하는 알고리즘이라, 셀 크기가 불규칙하거나
  격자가 아닌 지형에는 그대로 쓸 수 없습니다.

  또 세 축 모두에 대해 TDelta·TMax를 초기 계산하므로, 캐릭터의 XZ 평면
  이동처럼 두 축만 필요한 Ray도 3축 설정 비용을 냅니다. 실제 전진 단계에서
  Y축이 선택되지는 않지만, 초기 계산은 그대로 들어갑니다.

  마지막으로 지금 구현은 Ray가 그리드 바깥에서 시작하는 경우를 지원하지
  않습니다. 순회 도중 셀 인덱스가 범위를 벗어나면 `PosToIndex`가 음수를
  반환하고 즉시 `false`로 빠져나가기 때문에, 맵 밖에서 안쪽으로 쏘는 Ray는
  그리드 진입 지점을 따로 구해주지 않는 한 충돌을 놓칩니다. 현재 게임에서는
  Ray가 항상 맵 안의 캐릭터에서 출발해 문제가 드러나지 않았을 뿐, 구현상
  남아 있는 제약입니다.

## 대안 비교

동일 씬, Release x64, 10만 회 평균 기준입니다.

| | 전체 셀 순회 | 3D DDA (선택) |
|---|---|---|
| 검사 대상 | 맵의 모든 셀 | Ray가 지나는 셀만 |
| 셀 통과 순서 | 보장 안 됨 | 정확히 보장 |
| 시간복잡도 | O(맵 전체 셀 수) | O(Ray가 지나는 셀 수) ≈ O(Ray 길이) |
| 실측 평균 | 14.9μs | 0.2μs |

**전체 셀 순회 대비 약 98.7% 단축**됐습니다.

전체 셀 순회를 버린 이유는 속도만이 아닙니다. 통과 순서가 보장되지 않으니 가장
가까운 충돌을 찾으려면 어차피 전부 검사한 뒤 거리를 비교해야 했고, 맵이 커질수록
비용이 선형으로 늘어나는 구조라 맵 확장 자체가 막히는 문제가 있었습니다.

## 회고

그리드 바깥에서 시작하는 Ray를 지원하지 않는 건 지금이라도 고칠 수 있는
부분입니다. Ray와 그리드 AABB의 교차점을 먼저 구해 그 지점부터 순회를 시작하면
되는데, 당장 그런 Ray를 쏘는 곳이 없어서 미뤄뒀습니다.
