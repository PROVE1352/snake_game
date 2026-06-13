// GameController.h
// 게임 전체 흐름을 제어하는 클래스.
//   - 한 스테이지의 실행 루프(입력 수집 → 상태 갱신 → 화면 렌더링)를 담당
//   - 인트로 / 도움말 / 랭킹 보드 / 스테이지 클리어·게임오버·최종 성공 화면 관리
//   - Map·Snake·Item·Gate·BlockWall·ScoreBoard 를 소유하고 조율한다
//   - RankingManager 와 연동해 플레이 결과를 랭킹에 기록한다

#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <string>
#include <vector>
#include "Map.h"
#include "Snake.h"
#include "ScoreBoard.h"
#include "Gate.h"
#include "food.h"
#include "poison.h"
#include "speed_item.h"
#include "BlockWall.h"
#include "RankingManager.h"

// 게임 결과 표현
enum class GameResult
{
    STAGE_CLEAR,
    GAME_OVER,
    QUIT
};

// 단일 스테이지 실행 흐름과 UI 연출을 총괄하는 클래스
class GameController
{
public:
    // 생성자: 실행할 스테이지 번호와 해당 스테이지의 맵 파일 경로를 주입받음
    GameController(const int stageNum, const std::string &mapFilePath);
    ~GameController() = default;

    // 게임판을 로드하고 뱀의 초기 위치와 아이템, 게이트 설정 진행. 성공 시 true
    bool initialize();

    // 스테이지 메인 업데이트 루프(waitAndProcessInput -> update -> render)를 구동함
    GameResult run();

    // 인트로 화면 표시 (게임 시작 시 true, 종료 선택 시 false 반환)
    static bool showIntroScreen(RankingManager &rankingManager);

    // 도움말/조작법 화면 표시
    static void showHelpScreen();

    // 랭킹 보드 화면 표시
    // 계속 진행 A 입력 시 true, 메인으로 복귀 Q 입력 시 false
    static bool showRankingBoardScreen(RankingManager &rankingManager, const int initialStage, const std::string &bottomMessage, const bool allowSwitch);

    // 랭킹 테이블 그리기 헬퍼 함수
    static void drawRankingTable(const std::vector<RankingRecord> &ranks, const int stageFilter, const std::string &bottomMessage);

    // 스테이지 결과 화면 표시
    // 랭킹 보드 연동 (계속하기 A는 true, 나가기 Q는 false 반환)
    bool showStageClearScreen(RankingManager &rankingManager) const;

    // 게임오버 화면 표시 (다시하기 A는 true, 나가기 Q는 false 반환)
    bool showGameOverScreen(RankingManager &rankingManager) const;

    // 최종 성공 스크린
    // 랭킹 보드 연동 (계속하기 A는 true, 나가기 Q는 false 반환)
    static bool showTotalClearScreen(RankingManager &rankingManager, const int maxLength, const int growth, const int poison, const int speed, const int gate);

    // 스테이지 점수 조회
    // 최종 랭킹 기록용
    int getMaxLength() const { return scoreBoard.getMaxLength(); }
    int getGrowthCount() const { return scoreBoard.getGrowthCount(); }
    int getPoisonCount() const { return scoreBoard.getPoisonCount(); }
    int getSpeedCount() const { return scoreBoard.getSpeedCount(); }
    int getGateCount() const { return scoreBoard.getGateCount(); }

private:
    // 입력 수집 & 틱 시간 대기
    void waitAndProcessInput();

    // 뱀 이동, 아이템 수명, 게이트 업데이트, 장애물 상태, 미션 달성 여부 등을 업데이트함
    void update();

    // 갱신된 게임판(Map)과 성적 현황판(ScoreBoard)을 화면에 표시
    void render() const;

private:
    int stageNum;            // 현재 기동 중인 스테이지 번호 (1~5)
    std::string mapFilePath; // 실행할 맵 데이터 파일 경로

    Map map;               // 게임 맵 및 렌더링을 담당하는 맵 객체
    Snake snake;           // 뱀 위치와 이동 방향 제어 객체
    ScoreBoard scoreBoard; // 실시간 성적 수집 및 미션 관리 객체
    Gate gate;             // 게이트 텔레포트 관리 객체
    Food growthItem;       // 성장 아이템 관리 객체
    Poison poisonItem;     // 독약 아이템 관리 객체
    SpeedItem speedItem;   // 스피드 아이템 및 가속 상태 관리 객체
    BlockWall blockWall;   // 테트리스 모양 장애물 벽 관리 객체

    bool isRunning;  // 게임 루프가 중단 없이 활성화되어 동작 중인지 여부
    bool gameOver;   // 뱀이 벽이나 몸통과 충돌하여 게임오버 상태가 되었는지 여부
    bool stageClear; // 모든 스테이지 목표 미션을 충족하여 클리어 상태가 되었는지 여부
    bool userQuit;   // 사용자가 ESC 또는 Q 키를 눌러 스테이지를 포기했는지 여부
};

#endif
