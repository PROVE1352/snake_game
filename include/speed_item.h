// speed_item.h
// 뱀의 이동 속도를 2배(틱 주기 절반)로 가속하는 스피드 아이템(SPEED_ITEM) 관리
// 맵 상의 빈 공간(EMPTY) 중 무작위 위치를 골라 생성
// 획득 시 30틱 동안 가속 효과 지속 및 2배 속도 연산용 주기 보정 처리

#ifndef SPEED_ITEM_H
#define SPEED_ITEM_H

#include "common.h"

class Map;

// 스피드 아이템(Speed Item)의 위치, 스폰, 수명 및 뱀의 속도 가속 타이머를 제어하는 클래스
class SpeedItem
{
public:
    SpeedItem();

    // 매 틱 호출되어 아이템의 스폰 상태 및 타이머를 갱신
    void update(Map &map);

    // move() 직전에 호출하여 "맵에 있었는지"를 확인
    void prepareSpeed(const Map &map);

    // move() 직후에 호출.
    // 반환값: 이번 tick 에 Growth/Poison 아이템도 갱신해야 하면 true
    bool updateSpeed(Map &map);

    // Speed 효과가 지속 중인지 여부
    bool isSpeedActive() const;

    // 아이템 상태 초기화
    void reset();

    // 현재 아이템의 좌표 반환
    int getY() const { return y; }
    int getX() const { return x; }

    // 현재 아이템이 맵 상에 활성화되어 존재 중인지 여부 반환
    bool isActive() const { return active; }

private:
    int y;           // 아이템의 y 좌표
    int x;           // 아이템의 x 좌표
    bool active;     // 맵 활성화 여부 (true 면 맵 상에 배치)
    int timer;       // 맵에 배치된 이후 경과한 틱 수 (30틱이 되면 소멸 후 재배치)
    int effectTimer; // 스피드 가속 효과의 남은 지속 시간 (틱 수)
    int tickCounter; // 아이템 갱신 주기 카운터
    bool wasOnMap;   // move() 직전 맵 위 존재 여부

    // 빈 칸 하나를 무작위로 골라 아이템을 새로 배치
    void spawn(Map &map);
};

#endif
