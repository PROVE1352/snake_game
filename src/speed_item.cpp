// speed_item.cpp
// SpeedItem 클래스의 구현 파일.
//   - 스피드 아이템(SPEED_ITEM)의 스폰 및 뱀의 가속 상태(effectTimer) 관리
//   - 뱀이 스피드 아이템을 먹으면 30틱 동안 이동 속도를 2배로 가속함
//   - 2배 속도는 가속 중일 때 홀수 프레임의 이동 연산을 생략하고 짝수 프레임에만 2칸씩 움직이는 주기를 시뮬레이션함

#include "speed_item.h"
#include "Map.h"
#include <cstdlib>
#include <vector>

static const int ITEM_LIFETIME = 30;         // 아이템의 수명 (30틱)
static const int SPEED_EFFECT_DURATION = 30; // 가속 효과 지속 시간 (30틱)

// 생성자: 멤버 변수를 초기값으로 바인딩함
SpeedItem::SpeedItem()
    : y(0), x(0), active(false), timer(0),
      effectTimer(0), tickCounter(0), wasOnMap(false)
{
}

// 맵 상의 빈 공간(EMPTY)을 수집하여 임의의 위치에 스피드 아이템을 스폰함
void SpeedItem::spawn(Map &map)
{
    std::vector<Position> empties;
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
        return; // 빈 칸이 없으면 생성 포기

    const int idx = rand() % (int)empties.size();
    y = empties[idx].y;
    x = empties[idx].x;
    active = true;
    timer = ITEM_LIFETIME;
    map.setCell(y, x, SPEED_ITEM);
}

// 매 틱 호출되어 아이템의 수명을 깎고, 만료되거나 먹힌 경우 재배치함
void SpeedItem::update(Map &map)
{
    if (active == false)
    {
        spawn(map);
        return;
    }

    if (map.getCell(y, x) != SPEED_ITEM)
    {
        spawn(map);
        return;
    }

    timer--;
    if (timer <= 0)
    {
        map.setCell(y, x, EMPTY);
        spawn(map);
    }
}

// 이동 연산 전에 아이템이 필드에 정상적으로 존속 중이었는지 여부를 기록함 (획득 감지 버퍼링)
void SpeedItem::prepareSpeed(const Map &map)
{
    wasOnMap = active && (map.getCell(y, x) == SPEED_ITEM);
}

// 뱀 이동이 진행된 후 호출되어 가속 상태를 적용하고 프레임 생략 여부를 판정함
// 반환값: true = 속도 타이머 감소 진행(정상), false = 속도 효과로 인한 다음 프레임 생략 대상
bool SpeedItem::updateSpeed(Map &map)
{
    tickCounter++;
    // 가속 상태가 아닐 때는 매 프레임 정상 진행, 가속 상태인 경우에는 2프레임에 한 번씩만 게임 루프 진행
    const bool shouldUpdate = (effectTimer <= 0) || (tickCounter % 2 == 0);

    if (shouldUpdate)
    {
        // 필드 상에 존속했던 아이템이 이동 완료 후 소멸했다면 뱀이 획득한 것으로 간주함
        if (wasOnMap && map.getCell(y, x) != SPEED_ITEM)
        {
            effectTimer = SPEED_EFFECT_DURATION; // 가속 버프 타이머 설정
        }
        update(map); // 아이템 스폰/재배치 상태 갱신
    }

    // 버프 지속시간 차감
    if (effectTimer > 0)
    {
        effectTimer--;
    }

    return shouldUpdate;
}

// 스피드 가속 효과가 현재 지속 상태인지 반환함
bool SpeedItem::isSpeedActive() const
{
    return effectTimer > 0;
}

// 아이템 및 가속 버프 관련 모든 데이터 멤버를 초기화함
void SpeedItem::reset()
{
    y = 0;
    x = 0;
    active = false;
    timer = 0;
    effectTimer = 0;
    tickCounter = 0;
    wasOnMap = false;
}
