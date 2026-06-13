// poison.cpp
// Poison 클래스의 구현 파일.
//   - 독약 아이템(POISON_ITEM)의 스폰 위치 선정 및 수명 주기를 연산함
//   - 뱀이 아이템을 획득했거나 일정 시간이 경과하면 새로운 위치로 재스폰함

#include "poison.h"
#include "Map.h"
#include <cstdlib>
#include <vector>

static const int ITEM_LIFETIME = 30; // 아이템의 수명 (30틱)

// 생성자: 멤버 변수를 기본값으로 초기화함
Poison::Poison()
    : y(0), x(0), active(false), timer(0)
{
}

// 맵 상의 빈 공간(EMPTY)을 수집하여 임의의 위치에 독약 아이템을 스폰함
void Poison::spawn(Map &map)
{
    std::vector<Position> empties;
    // 맵 내의 모든 빈 칸 좌표를 수집
    for (int cy = 0; cy < map.getHeight(); cy++)
    {
        for (int cx = 0; cx < map.getWidth(); cx++)
        {
            if (map.getCell(cy, cx) == EMPTY)
            {
                Position p = {cy, cx};
                empties.push_back(p);
            }
        }
    }
    if (empties.empty())
        return; // 빈 칸이 전혀 없으면 생성 포기

    // 무작위 빈 칸을 선정하여 아이템 정보를 저장하고 맵 셀 값을 갱신함
    const int idx = rand() % (int)empties.size();
    y = empties[idx].y;
    x = empties[idx].x;
    active = true;
    timer = ITEM_LIFETIME;
    map.setCell(y, x, POISON_ITEM);
}

// 매 틱 호출되어 아이템의 수명을 깎고, 만료되거나 먹힌 경우 재배치함
void Poison::update(Map &map)
{
    // 활성화 상태가 아니면 즉각 스폰
    if (active == false)
    {
        spawn(map);
        return;
    }

    // 뱀에 의해 획득된 상태(맵에서 사라진 상태)인지 검사
    if (map.getCell(y, x) != POISON_ITEM)
    {
        spawn(map);
        return;
    }

    // 수명 타이머 감소 연산
    timer--;
    if (timer <= 0)
    {
        map.setCell(y, x, EMPTY); // 만료 시 기존 자리 제거
        spawn(map);               // 재생성
    }
}

// 아이템 상태를 초기 미배치 상태로 완전 리셋함
void Poison::reset()
{
    y = 0;
    x = 0;
    active = false;
    timer = 0;
}
