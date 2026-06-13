// Snake.cpp
// 뱀 클래스 구현

#include "Snake.h"
#include "Map.h"
#include "Gate.h"

// 생성자: 길이 0, 일단 오른쪽 방향
Snake::Snake()
{
    length = 0;
    dir = DIR_RIGHT;
    nextDir = DIR_RIGHT;
    dirQueue = std::queue<Direction>();
    for (int i = 0; i < SNAKE_MAX_LENGTH; i++)
    {
        body[i].y = 0;
        body[i].x = 0;
    }
}

// 맵에서 뱀 머리(3) 와 몸통(4) 위치를 찾아서 body 배열에 채움
// 머리 -> 인접한 몸통 -> 그 몸통의 인접한 몸통 ... 순으로 따라가며 등록
bool Snake::initFromMap(const Map &map)
{
    // 머리 위치 찾기
    int headY = -1;
    int headX = -1;
    for (int y = 0; y < map.getHeight(); y++)
    {
        for (int x = 0; x < map.getWidth(); x++)
        {
            if (map.getCell(y, x) == SNAKE_HEAD)
            {
                headY = y;
                headX = x;
            }
        }
    }
    if (headY < 0)
    {
        // 맵에 머리가 없음
        return false;
    }

    // body[0] = 머리
    body[0].y = headY;
    body[0].x = headX;
    length = 1;

    // 머리 위치에서 시작해서, 인접한 몸통을 따라가며 차례대로 등록
    int curY = headY;
    int curX = headX;
    while (length < SNAKE_MAX_LENGTH)
    {
        bool found = false;

        // 위 칸 확인
        if (found == false && map.getCell(curY - 1, curX) == SNAKE_BODY)
        {
            bool already = false;
            for (int i = 0; i < length; i++)
            {
                if (body[i].y == curY - 1 && body[i].x == curX)
                {
                    already = true;
                }
            }
            if (already == false)
            {
                body[length].y = curY - 1;
                body[length].x = curX;
                length++;
                curY = curY - 1;
                found = true;
            }
        }

        // 아래 칸 확인
        if (found == false && map.getCell(curY + 1, curX) == SNAKE_BODY)
        {
            bool already = false;
            for (int i = 0; i < length; i++)
            {
                if (body[i].y == curY + 1 && body[i].x == curX)
                {
                    already = true;
                }
            }
            if (already == false)
            {
                body[length].y = curY + 1;
                body[length].x = curX;
                length++;
                curY = curY + 1;
                found = true;
            }
        }

        // 왼쪽 칸 확인
        if (found == false && map.getCell(curY, curX - 1) == SNAKE_BODY)
        {
            bool already = false;
            for (int i = 0; i < length; i++)
            {
                if (body[i].y == curY && body[i].x == curX - 1)
                {
                    already = true;
                }
            }
            if (already == false)
            {
                body[length].y = curY;
                body[length].x = curX - 1;
                length++;
                curX = curX - 1;
                found = true;
            }
        }

        // 오른쪽 칸 확인
        if (found == false && map.getCell(curY, curX + 1) == SNAKE_BODY)
        {
            bool already = false;
            for (int i = 0; i < length; i++)
            {
                if (body[i].y == curY && body[i].x == curX + 1)
                {
                    already = true;
                }
            }
            if (already == false)
            {
                body[length].y = curY;
                body[length].x = curX + 1;
                length++;
                curX = curX + 1;
                found = true;
            }
        }

        // 더 이상 따라갈 몸통이 없으면 끝
        if (found == false)
        {
            break;
        }
    }

    // 초기 진행 방향 정하기
    // 머리(body[0])에서 바로 뒤(body[1])의 반대쪽 = 머리가 바라보는 쪽
    if (length >= 2)
    {
        const int dy = body[0].y - body[1].y;
        const int dx = body[0].x - body[1].x;
        if (dy == -1)
            dir = DIR_UP;
        else if (dy == 1)
            dir = DIR_DOWN;
        else if (dx == -1)
            dir = DIR_LEFT;
        else if (dx == 1)
            dir = DIR_RIGHT;
    }
    nextDir = dir;
    dirQueue = std::queue<Direction>();
    return true;
}

// 두 방향이 서로 반대인지 확인
bool Snake::isOpposite(const Direction a, const Direction b) const
{
    if (a == DIR_UP && b == DIR_DOWN)
        return true;
    if (a == DIR_DOWN && b == DIR_UP)
        return true;
    if (a == DIR_LEFT && b == DIR_RIGHT)
        return true;
    if (a == DIR_RIGHT && b == DIR_LEFT)
        return true;
    return false;
}

// 사용자가 방향키를 누르면 호출됨
void Snake::requestDirection(const Direction d)
{
    // 큐의 마지막 원소(큐가 비어있다면 현재 dir)를 기준으로 같은 방향이면 무시
    const Direction referenceDir = dirQueue.empty() ? dir : dirQueue.back();
    if (d == referenceDir)
    {
        return;
    }

    // 큐 크기 제한: 최대 2개로 제한
    // 큐가 가득 찼다면(크기 2) 가장 최근에 등록하려던(큐의 마지막 원소) 방향을 덮어씀
    if (dirQueue.size() >= 2)
    {
        dirQueue.back() = d;
    }
    else
    {
        dirQueue.push(d);
    }
}

// 뱀을 한 칸 움직임
int Snake::move(Map &map, Gate *const gate)
{
    if (!dirQueue.empty())
    {
        nextDir = dirQueue.front();
        dirQueue.pop();
    }
    else
    {
        nextDir = dir;
    }

    // 반대 방향 입력이면 게임 오버 (명세서: 반대 방향키 입력 시 실패)
    if (isOpposite(dir, nextDir) == true)
    {
        return -1;
    }
    // 이번 tick 부터 새 방향 적용
    dir = nextDir;

    // 다음 머리 위치 계산
    int newHeadY = body[0].y;
    int newHeadX = body[0].x;
    if (dir == DIR_UP)
    {
        newHeadY = newHeadY - 1;
    }
    else if (dir == DIR_DOWN)
    {
        newHeadY = newHeadY + 1;
    }
    else if (dir == DIR_LEFT)
    {
        newHeadX = newHeadX - 1;
    }
    else if (dir == DIR_RIGHT)
    {
        newHeadX = newHeadX + 1;
    }

    // 충돌 검사
    int target = map.getCell(newHeadY, newHeadX);

    // Gate 통과 처리: 머리가 GATE 셀에 진입하려 하면
    // 다른 Gate 너머 한 칸으로 텔레포트하고 진행 방향도 갱신한다
    if (target == GATE && gate != nullptr)
    {
        int teleY, teleX;
        Direction teleDir;
        if (gate->tryTeleport(newHeadY, newHeadX, dir, map,
                              teleY, teleX, teleDir) == false)
        {
            // 출구가 모두 막혀 있으면 통과 불가 -> 게임오버
            return -1;
        }
        newHeadY = teleY;
        newHeadX = teleX;
        dir = teleDir;
        nextDir = teleDir; // 진출 직후 사용자가 누른 키와 충돌 안 나도록 갱신
        target = map.getCell(newHeadY, newHeadX);
    }

    // 벽 충돌 (워프 흔적 USED_GATE_WALL, 블록 벽 BLOCK_WALL 도 동일하게 충돌)
    // BLOCK_WARN(예고)은 아직 벽이 아니므로 통과 가능 -> 충돌 목록에서 제외
    if (target == WALL || target == IMMUNE_WALL ||
        target == USED_GATE_WALL || target == BLOCK_WALL)
    {
        return -1;
    }
    // 자기 몸통 충돌
    // 꼬리(body[length-1])는 이번 tick 에 비워질 자리라서 제외
    for (int i = 0; i < length - 1; i++)
    {
        if (body[i].y == newHeadY && body[i].x == newHeadX)
        {
            return -1;
        }
    }

    // 이동 전의 target 셀 값을 반환하기 위해 보관
    const int result = target;

    // 아이템 종류에 따라 꼬리 처리 분기 설정
    if (target == GROWTH_ITEM)
    {
        length++; // 만약 자라나는 아이템을 먹었다면, 꼬리 증가
    }
    else
    {
        const int oldTailY = body[length - 1].y;
        const int oldTailX = body[length - 1].x;
        map.setCell(oldTailY, oldTailX, EMPTY);

        if (target == POISON_ITEM)
        {
            if (length - 1 < 3)
            {
                return -1; // RULE 2에 따라 몸의 길이가 3 이하면 실패
            }
            length--;
            // 독을 먹어서 잘려나간 꼬리 좌표 제거
            map.setCell(body[length - 1].y, body[length - 1].x, EMPTY);
        }
        // SPEED_ITEM은 길이 변화 없이 속도 변화만 줌
    }

    // 몸통을 한 칸씩 뒤로 밀기
    for (int i = length - 1; i > 0; i--)
    {
        body[i] = body[i - 1];
    }
    // 새 머리 위치
    body[0].y = newHeadY;
    body[0].x = newHeadX;

    // 맵에 새 위치 표시
    map.setCell(body[0].y, body[0].x, SNAKE_HEAD);
    if (length >= 2)
    {
        // 머리 바로 뒤 칸은 이제 몸통으로 표시
        map.setCell(body[1].y, body[1].x, SNAKE_BODY);
    }
    return result;
}
