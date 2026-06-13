// Map.h
// 게임판(board)을 담당하는 Map 클래스.
// 맵 데이터를 2차원 배열로 보관하고, 텍스트 파일에서 읽어오며,
// ncurses 화면에 색으로 그린다. 모든 단계가 이 Map을 공유한다.

#ifndef MAP_H
#define MAP_H

#include "common.h"

class Map
{
public:
    Map();

    // 텍스트 파일에서 맵을 읽어옴. 성공 시 true.
    bool loadFromFile(const char *const filename);

    // 현재 맵을 ncurses 화면에 그림 - offsetY, offsetX 부터 시작
    void draw(const int offsetY, const int offsetX) const;

    // 셀 값 읽기/쓰기
    int getCell(const int y, const int x) const;
    void setCell(const int y, const int x, const int value);
    int getHeight() const { return height; }
    int getWidth() const { return width; }

    // 5단계 미션 평가용. IMMUNE_WALL 제외, 맵 내부에 배치된 벽(WALL)의 개수를 셈
    int countInternalWalls() const;

    // ncurses 색 페어 초기화. 게임 시작할 때 한 번만 호출.
    void initColors() const;

private:
    int data[MAP_MAX_SIZE][MAP_MAX_SIZE];
    int height;
    int width;
};

#endif
