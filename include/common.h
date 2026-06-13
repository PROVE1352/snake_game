// common.h
// 프로젝트 전체가 공유하는 공용 정의 모음.
//   - enum Cell      : 맵 한 칸이 가질 수 있는 셀 값
//   - enum Direction : 뱀의 이동 방향
//   - 맵 크기·뱀 길이 등 상수
//   - struct Position: 좌표 한 쌍

#ifndef COMMON_H
#define COMMON_H

// 맵 셀 값
enum Cell
{
    EMPTY = 0,          // 빈 공간 (스폰 및 통과 가능)
    WALL = 1,           // 일반 벽 (충돌 시 게임오버, 게이트 출현 가능)
    IMMUNE_WALL = 2,    // 가장자리 벽 (충돌 시 게임오버, 게이트 출현 불가)
    SNAKE_HEAD = 3,     // 뱀의 머리
    SNAKE_BODY = 4,     // 뱀의 몸통 마디
    GROWTH_ITEM = 5,    // 몸 길이를 늘려주는 성장 아이템
    POISON_ITEM = 6,    // 몸 길이를 줄여주는 독약 아이템
    GATE = 7,           // 텔레포트 워프 게이트
    SPEED_ITEM = 8,     // 뱀 속도를 가속하는 스피드 아이템
    USED_GATE_WALL = 9, // 게이트가 생성되었다 닫힌 흔적 벽 (동일 자리 게이트 재생성 불가)
    BLOCK_WALL = 10,    // 임시 테트리스 모양 장애물 벽 (충돌 시 게임오버, 잠깐 나타났다 사라지는 벽)
    BLOCK_WARN = 11     // 임시 테트리스 모양 장애물 벽 스폰 1초 전 예고 표시 (통과 가능)
};

// 텍스트 출력용 전용 색 페어 ID
enum TextColorPair
{
    COLOR_PAIR_TEXT_GROWTH = 15,   // 초록색 (성장 미션 텍스트용)
    COLOR_PAIR_TEXT_POISON = 16,   // 빨간색 (독약 미션 텍스트용)
    COLOR_PAIR_TEXT_SPEED = 17,    // 하늘색 (스피드 미션 텍스트용)
    COLOR_PAIR_TEXT_GATE = 18,     // 보라색 (게이트 미션 텍스트용)
    COLOR_PAIR_TEXT_USED_GATE = 19 // 노란색 (워프 흔적 텍스트용)
};

// 이동 방향
enum Direction
{
    DIR_UP = 0, // 위 방향
    DIR_DOWN,   // 아래 방향
    DIR_LEFT,   // 왼쪽 방향
    DIR_RIGHT,  // 오른쪽 방향
    DIR_NONE    // 대기(정지) 상태
};

// 맵 최소 크기
const int MAP_MIN_SIZE = 21;

// 맵 최대 크기
const int MAP_MAX_SIZE = 30;

// 뱀 몸통 최대 길이
const int SNAKE_MAX_LENGTH = 100;

// 한 칸 = 화면 두 글자 폭
const int CELL_WIDTH = 2;

// 좌표 한 쌍
// 원래 Snake에 있었지만, item도 사용하기 위해 공용 헤더에 정의
struct Position
{
    int y;
    int x;
};

#endif
