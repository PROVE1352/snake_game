// ScoreBoard.h
// 현재 스테이지 진행 성적(뱀 길이, 획득한 아이템 수, 게이트 통과 수) 계측
// 각 항목별 완료해야 할 미션 목표치를 난이도 비례 동적 생성
// 미션 충족 여부를 실시간으로 화면에 표시

#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include "common.h"

// 게임 성적 집계와 스테이지 클리어 미션 상태 관리 및 화면 출력을 담당하는 클래스
class ScoreBoard
{
public:
    // 생성자: 스테이지 번호와 맵 내부 총 벽의 개수를 기준으로 미션 목표치를 자동 산정함
    ScoreBoard(const int stageNum, const int totalInternalWalls);

    // 아이템 획득 횟수를 1 증가시키고 미션 달성 여부 갱신
    void addGrowth();
    void addPoison();
    void addSpeed();
    void addGate();

    // 현재 뱀의 길이를 설정하고, 최장 길이(MaxLength) 갱신
    void updateLength(const int len);

    // 맵 내부에 존재하는 벽 개수를 갱신 (생성된 게이트 등에 의한 미션 가능성 체크용)
    void setInternalWalls(const int count) { currentInternalWalls = count; }

    // 현재 맵 구조상 게이트 생성 및 미션 달성이 가능한지 판정함
    bool canCompleteGateMission(const bool isGateActive) const;

    // 게이트 미션 달성 여부를 반환함
    bool isMissionGateComplete() const { return missionGate; }

    // 모든 미션(길이, 성장, 독약, 스피드, 게이트)을 달성했는지 여부를 반환함
    bool isAllMissionComplete() const;

    // 스테이지 점수 및 기록 조회
    int getMaxLength() const { return maxLength; }
    int getGrowthCount() const { return growthCount; }
    int getPoisonCount() const { return poisonCount; }
    int getSpeedCount() const { return speedCount; }
    int getGateCount() const { return gateCount; }

    // 스코어보드와 미션 현황판을 화면에 표시
    void draw(const int offsetY, const int offsetX) const;

private:
    int stage;                // 현재 스테이지 번호
    int currentLength;        // 현재 뱀의 길이
    int maxLength;            // 이번 스테이지에서 달성한 최장 뱀 길이
    int growthCount;          // 성장 아이템 획득 누적 개수
    int poisonCount;          // 독약 아이템 획득 누적 개수
    int speedCount;           // 스피드 아이템 획득 누적 개수
    int gateCount;            // 게이트 통과 누적 횟수
    int currentInternalWalls; // 맵 상에 존재하는 내부 벽의 개수

    int targetLength; // 목표 뱀 길이
    int targetGrowth; // 목표 성장 아이템 개수
    int targetPoison; // 목표 독약 아이템 개수
    int targetSpeed;  // 목표 스피드 아이템 개수
    int targetGate;   // 목표 게이트 통과 횟수

    bool missionLength; // 길이 미션 완료 여부
    bool missionGrowth; // 성장 미션 완료 여부
    bool missionPoison; // 독약 미션 완료 여부
    bool missionSpeed;  // 스피드 미션 완료 여부
    bool missionGate;   // 게이트 미션 완료 여부

    // 미션 달성 여부 실시간 체크
    void checkMissions();

    // 스테이지 난이도와 맵 내부 벽의 개수를 고려하여 적절한 미션 목표치 생성
    void generateMissions();
};

#endif
