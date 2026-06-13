// ScoreBoard.cpp
// ScoreBoard 클래스의 구현 파일.
//   - 현재 스테이지의 점수(길이, 획득한 성장/독약/스피드 아이템 수, 게이트 통과 수)를 관리
//   - 난이도(스테이지 단계)에 비례하여 클리어해야 할 미션 목표치를 랜덤 가중치와 함께 생성
//   - 점수판 및 미션 현황판을 화면에 표시
#include "ScoreBoard.h"
#include "curses_compat.h"
#include <cstdlib>
#include <locale.h>
#include <algorithm>

// 생성자: 각 카운터 변수를 초기화하고, 스테이지에 맞는 미션을 동적 생성
ScoreBoard::ScoreBoard(const int stageNum, const int totalInternalWalls)
{
    stage = stageNum;
    currentLength = 0;
    maxLength = 0;
    growthCount = 0;
    poisonCount = 0;
    speedCount = 0;
    gateCount = 0;
    currentInternalWalls = totalInternalWalls;

    missionLength = false;
    missionGrowth = false;
    missionPoison = false;
    missionSpeed = false;
    missionGate = false;

    generateMissions();
}

// 스테이지 단계 및 내부 벽 개수를 기반으로 실시간 점수 목표치를 계산함
void ScoreBoard::generateMissions()
{
    // 스테이지가 높아질수록 목표치가 커지도록 설정 - 난이도 조정!!
    const int base = stage * 2;
    // 목표 길이 범위 - S1:[5~7], S2:[7~9], S3:[9~11], S4:[11~13], S5:[13~15]
    targetLength = base + (rand() % 3) + 3;
    // 목표 성장 아이템 개수 범위 - S1:[2~4], S2:[4~6], S3:[6~8], S4:[8~10], S5:[10~12]
    targetGrowth = base + (rand() % 3);
    // 목표 독약 획득 횟수 범위 - S1:[1~3], S2:[2~4], S3:[2~4], S4:[3~5], S5:[3~5]
    targetPoison = (stage / 2) + (rand() % 3) + 1;
    // 목표 스피드 아이템 개수 범위 - S1:[2~4], S2:[3~5], S3:[4~6], S4:[5~7], S5:[6~8]
    targetSpeed = stage + (rand() % 3) + 1;
    // 목표 게이트 통과 횟수 범위 - S1:[1~2], S2:[2~3], S3:[3~4], S4:[4~5], S5:[5~6]
    targetGate = stage + (rand() % 2);
}

void ScoreBoard::addGrowth()
{
    growthCount++;
    checkMissions();
}

void ScoreBoard::addPoison()
{
    poisonCount++;
    checkMissions();
}

void ScoreBoard::addSpeed()
{
    speedCount++;
    checkMissions();
}

void ScoreBoard::addGate()
{
    gateCount++;
    checkMissions();
}

// 현재 내부 벽 개수를 바탕으로 남은 틱 동안 게이트 미션 달성이 물리적으로 가능한지 평가함
bool ScoreBoard::canCompleteGateMission(const bool isGateActive) const
{
    if (missionGate)
    {
        return true;
    }

    // 맵 상에 한 번에 존재할 수 있는 게이트 수와 필요한 잔여 통과 횟수를 비교
    const int potentialGates = (currentInternalWalls / 2) + (isGateActive ? 1 : 0);
    const int needed = targetGate - gateCount;
    return potentialGates >= needed;
}

// 뱀의 현재 길이를 설정하고, 스테이지 최장 기록을 저장한 후 미션을 판정함
void ScoreBoard::updateLength(const int len)
{
    currentLength = len;
    if (currentLength > maxLength)
    {
        maxLength = currentLength;
    }
    checkMissions();
}

// 획득 점수와 목표치를 실시간 대조하여 각 미션의 달성 여부(참/거짓)를 판정함
void ScoreBoard::checkMissions()
{
    missionLength = (maxLength >= targetLength); // 최대길이로 사용(난이도 이슈)
    missionGrowth = (growthCount >= targetGrowth);
    missionPoison = (poisonCount >= targetPoison);
    missionSpeed = (speedCount >= targetSpeed);
    missionGate = (gateCount >= targetGate);
}

// 5개 모든 미션 조건이 완료 상태를 만족하는지 판정함
bool ScoreBoard::isAllMissionComplete() const
{
    return missionLength && missionGrowth && missionPoison && missionSpeed && missionGate;
}

// 스코어보드, 미션 현황, 간단한 설명 창을 화면에 출력
void ScoreBoard::draw(const int offsetY, const int offsetX) const
{
    // 1. STAGE SCOREBOARD 타이틀
    attron(A_BOLD | COLOR_PAIR(COLOR_PAIR_TEXT_SPEED));
    mvprintw(offsetY, offsetX, "╔══════════════════════════╗");
    mvprintw(offsetY + 1, offsetX, "║    STAGE %d SCOREBOARD    ║", stage);
    mvprintw(offsetY + 2, offsetX, "╠══════════════════════════╣");
    attroff(A_BOLD | COLOR_PAIR(COLOR_PAIR_TEXT_SPEED));

    mvprintw(offsetY + 3, offsetX, "║  Length  : %-2d / (Max:%2d) ║", currentLength, maxLength);

    mvprintw(offsetY + 4, offsetX, "║  ");
    attron(COLOR_PAIR(GROWTH_ITEM));
    mvprintw(offsetY + 4, offsetX + 3, "  ");
    attroff(COLOR_PAIR(GROWTH_ITEM));
    mvprintw(offsetY + 4, offsetX + 5, " Growth : %-2d          ║", growthCount);

    mvprintw(offsetY + 5, offsetX, "║  ");
    attron(COLOR_PAIR(POISON_ITEM));
    mvprintw(offsetY + 5, offsetX + 3, "  ");
    attroff(COLOR_PAIR(POISON_ITEM));
    mvprintw(offsetY + 5, offsetX + 5, " Poison : %-2d          ║", poisonCount);

    mvprintw(offsetY + 6, offsetX, "║  ");
    attron(COLOR_PAIR(SPEED_ITEM));
    mvprintw(offsetY + 6, offsetX + 3, "  ");
    attroff(COLOR_PAIR(SPEED_ITEM));
    mvprintw(offsetY + 6, offsetX + 5, " Speed  : %-2d          ║", speedCount);

    mvprintw(offsetY + 7, offsetX, "║  ");
    attron(COLOR_PAIR(GATE));
    mvprintw(offsetY + 7, offsetX + 3, "  ");
    attroff(COLOR_PAIR(GATE));
    mvprintw(offsetY + 7, offsetX + 5, " Gate   : %-2d (Max:%2d) ║", gateCount, currentInternalWalls / 2);

    mvprintw(offsetY + 8, offsetX, "╚══════════════════════════╝");

    // 2. MISSIONS 현황
    const int mOffset = offsetY + 10;
    attron(A_BOLD | COLOR_PAIR(COLOR_PAIR_TEXT_USED_GATE));
    mvprintw(mOffset, offsetX, "╔══════════════════════════╗");
    mvprintw(mOffset + 1, offsetX, "║         MISSIONS         ║");
    mvprintw(mOffset + 2, offsetX, "╠══════════════════════════╣");
    attroff(A_BOLD | COLOR_PAIR(COLOR_PAIR_TEXT_USED_GATE));

    // 미션 항목 한 행을 출력하는 람다 헬퍼 함수
    auto printMissionRow = [&](const int row, const int cellType, const char *const labelStr, const int current, const int target, const bool complete)
    {
        mvprintw(mOffset + row, offsetX, "║  ");
        if (cellType > 0)
        {
            attron(COLOR_PAIR(cellType));
            mvprintw(mOffset + row, offsetX + 3, "  ");
            attroff(COLOR_PAIR(cellType));
            mvprintw(mOffset + row, offsetX + 5, " %s : %2d / %-2d", labelStr, current, target);
        }
        else
        {
            mvprintw(mOffset + row, offsetX + 3, "Length : %2d / %-2d   ", current, target);
        }

        if (complete)
        {
            attron(COLOR_PAIR(COLOR_PAIR_TEXT_GROWTH) | A_BOLD);
            mvprintw(mOffset + row, offsetX + 23, "[V]");
            attroff(COLOR_PAIR(COLOR_PAIR_TEXT_GROWTH) | A_BOLD);
        }
        else
        {
            attron(COLOR_PAIR(COLOR_PAIR_TEXT_POISON));
            mvprintw(mOffset + row, offsetX + 23, "[ ]");
            attroff(COLOR_PAIR(COLOR_PAIR_TEXT_POISON));
        }
        mvprintw(mOffset + row, offsetX + 27, "║");
    };

    printMissionRow(3, 0, "Length", maxLength, targetLength, missionLength);
    printMissionRow(4, GROWTH_ITEM, "Growth", growthCount, targetGrowth, missionGrowth);
    printMissionRow(5, POISON_ITEM, "Poison", poisonCount, targetPoison, missionPoison);
    printMissionRow(6, SPEED_ITEM, "Speed ", speedCount, targetSpeed, missionSpeed);
    printMissionRow(7, GATE, "Gate  ", gateCount, targetGate, missionGate);
    mvprintw(mOffset + 8, offsetX, "╚══════════════════════════╝");

    // 3. 조작법 도움말 설명 출력
    const int hOffset = mOffset + 10;
    attron(A_DIM);
    mvprintw(hOffset, offsetX, "╔══════════════════════════╗");
    mvprintw(hOffset + 1, offsetX, "║      조작법 및 정보      ║");
    mvprintw(hOffset + 2, offsetX, "╠══════════════════════════╣");
    mvprintw(hOffset + 3, offsetX, "║  방향키: 스네이크 이동   ║");
    mvprintw(hOffset + 4, offsetX, "║  Q 키  : 스테이지 종료   ║");
    mvprintw(hOffset + 5, offsetX, "║                          ║");
    mvprintw(hOffset + 6, offsetX, "║  ! 충돌 또는 길이 3 미만 ║");
    mvprintw(hOffset + 7, offsetX, "║    게임오버 처리됨       ║");
    mvprintw(hOffset + 8, offsetX, "╚══════════════════════════╝");
    attroff(A_DIM);
}