// food.h
// 뱀의 몸통 길이를 1 증가시키는 성장 아이템(GROWTH_ITEM) 관리
// 맵 상의 빈 공간(EMPTY) 중 무작위 위치를 골라 생성
// 일정 수명(30틱)이 지나거나 뱀이 획득하면 다른 빈 공간에 재배치

#ifndef FOOD_H
#define FOOD_H

#include "common.h"

class Map;

// 성장 아이템(Growth Item)의 위치, 스폰, 수명 주기를 제어하는 클래스
class Food
{
public:
    Food();

    // 매 틱 호출되어 아이템의 수명을 감소시키고, 만료 시 새로운 위치에 스폰
    void update(Map &map);

    // 아이템 상태를 초기 상태(미활성, 타이머 0) 리셋
    void reset();

    // 현재 아이템의 좌표 반환
    int getY() const { return y; }
    int getX() const { return x; }

    // 현재 아이템이 맵 상에 활성화되어 존재 중인지 여부 반환
    bool isActive() const { return active; }

private:
    int y;       // 아이템의 y 좌표
    int x;       // 아이템의 x 좌표
    bool active; // 맵 활성화 여부 (true 면 맵 상에 배치됨)
    int timer;   // 맵에 배치된 이후 경과한 틱 수 (30틱이 되면 소멸 후 재배치)

    // 빈 칸 하나를 무작위로 골라 아이템을 새로 배치
    void spawn(Map &map);
};

#endif
