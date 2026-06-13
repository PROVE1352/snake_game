// GameController.cpp
// GameController 클래스의 구현 파일.
//   - 스테이지 구동 초기화 및 메인 게임 루프(프로세스 입력 -> 상태 업데이트 -> 화면 렌더링) 구동
//   - 조작 조율: Map, Snake, Food, Poison, SpeedItem, Gate, BlockWall, ScoreBoard 통합 처리
//   - 화면 전환: 인트로 대기, 도움말 안내 팝업, 랭킹 스코어보드 집계 및 텍스트 파일 저장 연계

#include "GameController.h"
#include "curses_compat.h"
#include <cstdlib>
#include <algorithm>

static const int TICK_USEC = 200000;

static int getUtf8VisualWidth(const std::string &str)
{
    int width = 0;
    for (size_t i = 0; i < str.length();)
    {
        const unsigned char c = str[i];
        if (c < 0x80)
        {
            width += 1;
            i += 1;
        }
        else if ((c & 0xE0) == 0xC0)
        {
            width += 2;
            i += 2;
        }
        else if ((c & 0xF0) == 0xE0)
        {
            width += 2;
            i += 3;
        }
        else
        {
            i += 1;
        }
    }
    return width;
}

// 생성자 -> 스테이지 번호와 맵 파일 경로를 받아 멤버를 초기화 리스트로 설정
// 실제 맵 로드, 뱀 배치는 initialize() 에서 수행
GameController::GameController(const int stageNum, const std::string &mapFilePath)
    : stageNum(stageNum), mapFilePath(mapFilePath), map(), snake(), scoreBoard(stageNum, 0),
      gate(), growthItem(), poisonItem(), speedItem(), blockWall(),
      isRunning(false), gameOver(false), stageClear(false), userQuit(false) {}

// 스테이지 관련 멤버 객체들을 초기화하여 플레이 준비를 완료함
bool GameController::initialize()
{
    // 1. 맵 데이터 로드
    if (map.loadFromFile(mapFilePath.c_str()) == false)
    {
        return false;
    }
    map.initColors();

    // 2. 맵 내 뱀 초기 배치 정보 적립
    if (snake.initFromMap(map) == false)
    {
        return false;
    }

    // 3. 점수판 미션 할당 및 뱀 최초 길이 설정
    scoreBoard = ScoreBoard(stageNum, map.countInternalWalls());
    scoreBoard.updateLength(snake.getLength());

    // 4. 아이템 및 특수 장애물 멤버 객체 리셋
    growthItem.reset();
    poisonItem.reset();
    speedItem.reset();
    blockWall = BlockWall();

    isRunning = true;
    gameOver = false;
    stageClear = false;
    userQuit = false;

    nodelay(stdscr, TRUE); // getch 비블로킹 입력 모드 활성화

    return true;
}

// 단일 스테이지 게임 실행 흐름 관리
GameResult GameController::run()
{
    while (isRunning && !gameOver && !stageClear)
    {
        render();              // 1. 화면 렌더링
        waitAndProcessInput(); // 2. 대기 및 키 수집
        if (userQuit)
        {
            return GameResult::QUIT; // 사용자가 도중에 Q 키를 눌러 중단한 경우
        }
        update(); // 3. 게임 물리 상태 갱신
    }

    if (gameOver)
    {
        return GameResult::GAME_OVER; // 패배
    }
    return GameResult::STAGE_CLEAR; // 스테이지 완료 성공
}

// 틱 지연 시간 동안 대기하며 키 입력을 수집함
void GameController::waitAndProcessInput()
{
    // 스피드 아이템 버프 활성화 시 대기 틱 주기를 절반으로 줄여 2배 빠르게 기동
    const int waitTime = speedItem.isSpeedActive() ? TICK_USEC / 2 : TICK_USEC;
    int elapsed = 0;
    const int pollInterval = 10000; // 0.01초 간격으로 세분화해서 키 입력 폴링

    while (elapsed < waitTime)
    {
        const int key = getch();
        if (key != ERR)
        {
            if (key == 'q' || key == 'Q')
            {
                isRunning = false;
                userQuit = true;
                break;
            }
            else if (key == KEY_UP)
                snake.requestDirection(DIR_UP);
            else if (key == KEY_DOWN)
                snake.requestDirection(DIR_DOWN);
            else if (key == KEY_LEFT)
                snake.requestDirection(DIR_LEFT);
            else if (key == KEY_RIGHT)
                snake.requestDirection(DIR_RIGHT);
        }
        sleep_usec(pollInterval);
        elapsed += pollInterval;
    }
}

// 매 틱마다 호출되어 뱀의 움직임, 게이트 생성 여부, 테트리스 벽, 아이템 상태 등을 갱신함
void GameController::update()
{
    // 1. 스피드 아이템 획득 여부를 미리 판정하기 위한 상태 스냅샷 저장
    speedItem.prepareSpeed(map);

    // 2. 맵 내에 게이트 통과 미션을 완수하기에 충분한 일반 벽(WALL) 개수가 남아있는지 검사함
    scoreBoard.setInternalWalls(map.countInternalWalls());
    if (!scoreBoard.canCompleteGateMission(gate.isActive()))
    {
        // 남은 일반 벽이 부족해 게이트 미션 달성이 논리적으로 불가능하다면 충돌 없이도 즉각 스테이지 클리어 실패 처리
        gameOver = true;
        clear();

        const int totalWidth = map.getWidth() * CELL_WIDTH + 4 + 28;

        // 상단 가로 경계 상자 박스 틀 렌더링
        attron(COLOR_PAIR(COLOR_PAIR_TEXT_SPEED) | A_BOLD);
        mvprintw(0, 0, "╔");
        for (int x = 1; x < totalWidth - 1; x++)
        {
            mvprintw(0, x, "═");
        }
        mvprintw(0, totalWidth - 1, "╗");

        mvprintw(1, 0, "║");
        for (int x = 1; x < totalWidth - 1; x++)
        {
            mvprintw(1, x, " ");
        }
        mvprintw(1, totalWidth - 1, "║");

        mvprintw(2, 0, "╚");
        for (int x = 1; x < totalWidth - 1; x++)
        {
            mvprintw(2, x, "═");
        }
        mvprintw(2, totalWidth - 1, "╝");
        attroff(COLOR_PAIR(COLOR_PAIR_TEXT_SPEED) | A_BOLD);

        attron(COLOR_PAIR(COLOR_PAIR_TEXT_GROWTH) | A_BOLD);
        mvprintw(1, 2, "SNAKE GAME");
        attroff(COLOR_PAIR(COLOR_PAIR_TEXT_GROWTH) | A_BOLD);

        attron(COLOR_PAIR(COLOR_PAIR_TEXT_SPEED) | A_BOLD);
        mvprintw(1, totalWidth - 14, "[ STAGE %d ]", stageNum);
        attroff(COLOR_PAIR(COLOR_PAIR_TEXT_SPEED) | A_BOLD);

        // 마지막 맵 상태와 점수판을 렌더
        map.draw(3, 0);
        scoreBoard.draw(3, map.getWidth() * CELL_WIDTH + 4);
        mvprintw(3 + map.getHeight() + 1, 0, "Mission Impossible! 게이트 생성을 위한 벽이 부족합니다.");
        refresh();
        sleep_usec(1500000); // 사용자 안내를 위해 1.5초 대기
        return;
    }

    // 3. 게이트와 테트리스 블록 장벽의 위치 및 페이즈 갱신
    const int prevGateUseCount = gate.getUseCount();
    gate.update(map, snake);
    blockWall.update(map, snake);

    // 4. 뱀 한 틱 이동 처리
    const int target = snake.move(map, &gate);

    // 게이트 이용 횟수 증가 여부 체크 후 스코어 기록
    if (gate.getUseCount() > prevGateUseCount)
    {
        scoreBoard.addGate();
    }

    // 5. 충돌(target == -1) 감지 시 게임오버 전환, 정상 이동 시 아이템 획득 체크 및 가속 타이머 연동
    if (target == -1)
    {
        gameOver = true;
    }
    else
    {
        if (target == GROWTH_ITEM)
            scoreBoard.addGrowth();
        else if (target == POISON_ITEM)
            scoreBoard.addPoison();
        else if (target == SPEED_ITEM)
            scoreBoard.addSpeed();

        scoreBoard.updateLength(snake.getLength());

        // 가속 타이머 갱신 및 2배 속도 연산 시 홀수 프레임의 아이템 스폰 연산 생략 여부 처리
        const bool shouldUpdate = speedItem.updateSpeed(map);
        if (shouldUpdate)
        {
            growthItem.update(map);
            poisonItem.update(map);
        }

        // 6. 모든 미션 클리어 요건 완성 시 스테이지 클리어 처리
        if (scoreBoard.isAllMissionComplete())
        {
            stageClear = true;
        }
    }
}

// 화면 전체를 지우고 최신 맵 상태(Map)와 스코어 현황(ScoreBoard)을 배치 렌더링함
void GameController::render() const
{
    clear();

    const int totalWidth = map.getWidth() * CELL_WIDTH + 4 + 28;

    // 상단 스테이지 헤더 박스 테두리 렌더링
    attron(COLOR_PAIR(COLOR_PAIR_TEXT_SPEED) | A_BOLD);
    mvprintw(0, 0, "╔");
    for (int x = 1; x < totalWidth - 1; x++)
    {
        mvprintw(0, x, "═");
    }
    mvprintw(0, totalWidth - 1, "╗");

    mvprintw(1, 0, "║");
    for (int x = 1; x < totalWidth - 1; x++)
    {
        mvprintw(1, x, " ");
    }
    mvprintw(1, totalWidth - 1, "║");

    mvprintw(2, 0, "╚");
    for (int x = 1; x < totalWidth - 1; x++)
    {
        mvprintw(2, x, "═");
    }
    mvprintw(2, totalWidth - 1, "╝");
    attroff(COLOR_PAIR(COLOR_PAIR_TEXT_SPEED) | A_BOLD);

    attron(COLOR_PAIR(COLOR_PAIR_TEXT_GROWTH) | A_BOLD);
    mvprintw(1, 2, "SNAKE GAME");
    attroff(COLOR_PAIR(COLOR_PAIR_TEXT_GROWTH) | A_BOLD);

    attron(COLOR_PAIR(COLOR_PAIR_TEXT_SPEED) | A_BOLD);
    mvprintw(1, totalWidth - 14, "[ STAGE %d ]", stageNum);
    attroff(COLOR_PAIR(COLOR_PAIR_TEXT_SPEED) | A_BOLD);

    // 맵 데이터 렌더 및 스코어보드 패널 렌더
    map.draw(3, 0);
    scoreBoard.draw(3, map.getWidth() * CELL_WIDTH + 4);
    refresh();
}

// 게임 타이틀과 메인 메뉴 인트로 스크린을 띄움 (게임 구동 시 true, 종료 선택 시 false 반환)
bool GameController::showIntroScreen(const RankingManager &rankingManager)
{
    nodelay(stdscr, FALSE); // 메뉴 입력 조작을 위해 키를 블로킹 모드로 수집
    bool needRedraw = true;
    while (true)
    {
        if (needRedraw)
        {
            clear();
            int H, W;
            getmaxyx(stdscr, H, W);

            // 콘솔창 중앙 정렬을 위한 좌표 산출
            const int startY = (H > 22) ? (H - 22) / 2 : 0;
            const int startX = (W > 82) ? (W - 82) / 2 : 0;

            // 아스키 아트 타이틀 로고 출력
            attron(COLOR_PAIR(COLOR_PAIR_TEXT_GROWTH) | A_BOLD);
            mvprintw(startY, startX, "  ██████  ███    ██  █████  ██   ██ ███████      ██████   █████  ███    ███ ███████");
            mvprintw(startY + 1, startX, " ██       ████   ██ ██   ██ ██  ██  ██          ██       ██   ██ ████  ████ ██     ");
            mvprintw(startY + 2, startX, "  █████   ██ ██  ██ ███████ █████   █████       ██   ███ ███████ ██ ████ ██ █████  ");
            mvprintw(startY + 3, startX, "      ██  ██  ██ ██ ██   ██ ██  ██  ██          ██    ██ ██   ██ ██  ██  ██ ██     ");
            mvprintw(startY + 4, startX, " ██████   ██   ████ ██   ██ ██   ██ ███████      ██████  ██   ██ ██      ██ ███████");
            attroff(COLOR_PAIR(COLOR_PAIR_TEXT_GROWTH) | A_BOLD);

            const int boxX = startX + 11;
            const int boxY = startY + 7;

            // 메뉴 상자 렌더
            attron(COLOR_PAIR(COLOR_PAIR_TEXT_SPEED) | A_BOLD);
            mvprintw(boxY, boxX, "╔══════════════════════════════════════════════════════════╗");
            mvprintw(boxY + 1, boxX, "║                     MAIN MENU SELECT                     ║");
            mvprintw(boxY + 2, boxX, "╠══════════════════════════════════════════════════════════╣");
            attroff(COLOR_PAIR(COLOR_PAIR_TEXT_SPEED) | A_BOLD);

            mvprintw(boxY + 3, boxX, "║  [ A ] 키를 눌러 게임 시작                               ║");
            mvprintw(boxY + 4, boxX, "║  [ B ] 키를 눌러 랭킹 확인                               ║");
            mvprintw(boxY + 5, boxX, "║  [ ? ] 키를 눌러 도움말 및 조작법                        ║");
            mvprintw(boxY + 6, boxX, "║  [ Q ] 키를 눌러 게임 종료                               ║");
            mvprintw(boxY + 7, boxX, "╚══════════════════════════════════════════════════════════╝");

            attron(A_DIM);
            mvprintw(boxY + 9, boxX + 6, "2026 C++ Snake Game Project - 1조");
            attroff(A_DIM);

            refresh();
            needRedraw = false;
        }

        const int key = getch();
        if (key == ERR)
        {
            sleep_usec(10000);
            continue;
        }
        if (key == 'a' || key == 'A')
        {
            nodelay(stdscr, TRUE); // 인게임 진입 시 다시 비블로킹 폴링 세팅
            return true;
        }
        else if (key == 'q' || key == 'Q')
        {
            return false;
        }
        else if (key == 'b' || key == 'B')
        {
            // B 키 입력 시 랭킹 창 진입
            showRankingBoardScreen(rankingManager, 0, "[← / →] 방향키를 입력해 다른 스테이지의 랭킹을 확인할 수 있습니다. (돌아가기: [ A ])", true);
            needRedraw = true; // 복귀 시 다시 메뉴 화면 드로잉 지시
        }
        else if (key == '?' || key == 'h' || key == 'H')
        {
            // 도움말 창 팝업
            showHelpScreen();
            needRedraw = true;
        }
        else if (key == KEY_RESIZE)
        {
            needRedraw = true;
        }
    }
}

// 상세 게임 규칙, 조작법 및 아이템 종류 범례 안내 스크린 팝업 출력
void GameController::showHelpScreen()
{
    nodelay(stdscr, FALSE);
    clear();
    int H, W;
    getmaxyx(stdscr, H, W);
    (void)H;

    const int startX = (W > 70) ? (W - 70) / 2 : 0;
    const int startY = 1;

    // 안내판 상자 렌더
    attron(COLOR_PAIR(COLOR_PAIR_TEXT_USED_GATE) | A_BOLD);
    mvprintw(startY, startX, "╔════════════════════════════════════════════════════════════════════╗");
    mvprintw(startY + 1, startX, "║                        HELP & GAME CONTROLS                        ║");
    mvprintw(startY + 2, startX, "╠════════════════════════════════════════════════════════════════════╣");
    attroff(COLOR_PAIR(COLOR_PAIR_TEXT_USED_GATE) | A_BOLD);

    mvprintw(startY + 3, startX, "║  - 조작법                                                          ║");
    mvprintw(startY + 4, startX, "║    - 방향키 [ ↑ | ↓ | ← | → ] : 뱀 이동 방향 전환                  ║");
    mvprintw(startY + 5, startX, "║    - [ Q ] 키 : 게임 중 강제 종료 및 이전 화면 이동                ║");
    mvprintw(startY + 6, startX, "║                                                                    ║");

    mvprintw(startY + 7, startX, "║  - 게임 아이템 종류                                                ║");

    mvprintw(startY + 8, startX, "║    - ");
    attron(COLOR_PAIR(GROWTH_ITEM));
    printw("  ");
    attroff(COLOR_PAIR(GROWTH_ITEM));
    attron(COLOR_PAIR(COLOR_PAIR_TEXT_GROWTH));
    printw(" Growth Item (초록색) : 뱀의 몸통 길이 1 증가             ");
    attroff(COLOR_PAIR(COLOR_PAIR_TEXT_GROWTH));
    mvprintw(startY + 8, startX + 69, "║");

    mvprintw(startY + 9, startX, "║    - ");
    attron(COLOR_PAIR(POISON_ITEM));
    printw("  ");
    attroff(COLOR_PAIR(POISON_ITEM));
    attron(COLOR_PAIR(COLOR_PAIR_TEXT_POISON));
    printw(" Poison Item (빨간색) : 뱀의 몸통 길이 1 감소             ");
    attroff(COLOR_PAIR(COLOR_PAIR_TEXT_POISON));
    mvprintw(startY + 9, startX + 69, "║");

    mvprintw(startY + 10, startX, "║    - ");
    attron(COLOR_PAIR(SPEED_ITEM));
    printw("  ");
    attroff(COLOR_PAIR(SPEED_ITEM));
    attron(COLOR_PAIR(COLOR_PAIR_TEXT_SPEED));
    printw(" Speed Item  (시안색) : 뱀의 이동속도 2배 (30틱간 지속)   ");
    attroff(COLOR_PAIR(COLOR_PAIR_TEXT_SPEED));
    mvprintw(startY + 10, startX + 69, "║");

    mvprintw(startY + 11, startX, "║    - ");
    attron(COLOR_PAIR(GATE));
    printw("  ");
    attroff(COLOR_PAIR(GATE));
    attron(COLOR_PAIR(COLOR_PAIR_TEXT_GATE));
    printw(" Warp Gate   (자홍색) : 머리 진입 시 반대편 게이트 워프   ");
    attroff(COLOR_PAIR(COLOR_PAIR_TEXT_GATE));
    mvprintw(startY + 11, startX + 69, "║");

    mvprintw(startY + 12, startX, "║    - ");
    attron(COLOR_PAIR(USED_GATE_WALL));
    printw("  ");
    attroff(COLOR_PAIR(USED_GATE_WALL));
    attron(COLOR_PAIR(COLOR_PAIR_TEXT_USED_GATE));
    printw(" Used Wall   (노란색) : 한 번 사용된 워프 흔적 (재이용 불가)");
    attroff(COLOR_PAIR(COLOR_PAIR_TEXT_USED_GATE));
    mvprintw(startY + 12, startX + 69, "║");

    mvprintw(startY + 13, startX, "║                                                                    ║");
    mvprintw(startY + 14, startX, "║  - 게임 규칙 및 실패 조건                                          ║");
    mvprintw(startY + 15, startX, "║    - 각 스테이지별 우측 미션 목표를 전부 완수 시 클리어            ║");
    mvprintw(startY + 16, startX, "║    - 뱀의 머리가 벽에 충돌 시 실패                                 ║");
    mvprintw(startY + 17, startX, "║    - 독약을 먹고 뱀의 몸통 길이가 3 미만이 될 시 실패 (게임 오버)  ║");
    mvprintw(startY + 18, startX, "║    - 맵 내 남은 벽이 부족해져 게이트 미션 달성이 불가할 시 실패    ║");
    mvprintw(startY + 19, startX, "╚════════════════════════════════════════════════════════════════════╝");

    attron(A_BOLD | COLOR_PAIR(COLOR_PAIR_TEXT_SPEED));
    mvprintw(startY + 21, startX + 14, "[ A ] 키를 누르면 이전 시작화면으로 이동합니다.");
    attroff(A_BOLD | COLOR_PAIR(COLOR_PAIR_TEXT_SPEED));

    refresh();

    // 사용자가 A 또는 A 대소문자를 눌러 창을 닫을 때까지 무한 대기
    while (true)
    {
        const int key = getch();
        if (key == ERR)
        {
            sleep_usec(10000);
            continue;
        }
        if (key == 'a' || key == 'A')
        {
            break;
        }
    }
}

// 랭킹 보드 스크린 조작 루프 (allowSwitch가 참이면 좌우 방향키로 스테이지별 필터 전환 가능)
bool GameController::showRankingBoardScreen(const RankingManager &rankingManager, const int initialStage, const std::string &bottomMessage, const bool allowSwitch)
{
    nodelay(stdscr, FALSE);
    int stageFilter = initialStage;
    bool needRedraw = true;

    while (true)
    {
        if (needRedraw)
        {
            // 1. 해당 스테이지 성적을 정렬하여 상위 10등 리스트 취득
            const auto ranks = rankingManager.getRankings(stageFilter);
            // 2. 테이블 드로잉
            drawRankingTable(ranks, stageFilter, bottomMessage);
            needRedraw = false;
        }

        const int key = getch();
        if (key == ERR)
        {
            sleep_usec(10000);
            continue;
        }
        if (key == 'a' || key == 'A')
        {
            return true; // 계속 게임 진행 선택
        }
        else if (key == 'q' || key == 'Q')
        {
            return false; // 복귀 또는 중단 선택
        }
        else if (allowSwitch)
        {
            // 방향키 입력 시 스테이지 필터 순환 변경
            if (key == KEY_LEFT)
            {
                stageFilter--;
                if (stageFilter < 0)
                    stageFilter = 5;
                needRedraw = true;
            }
            else if (key == KEY_RIGHT)
            {
                stageFilter++;
                if (stageFilter > 5)
                    stageFilter = 0;
                needRedraw = true;
            }
        }
        else if (key == KEY_RESIZE)
        {
            needRedraw = true;
        }
    }
}

// 랭킹 데이터를 포맷에 맞게 조립하여 박스 선과 함께 드로잉함
void GameController::drawRankingTable(const std::vector<RankingRecord> &ranks, const int stageFilter, const std::string &bottomMessage)
{
    clear();
    int H, W;
    getmaxyx(stdscr, H, W);
    (void)H;

    const int boxWidth = 88;
    const int startX = (W > boxWidth) ? (W - boxWidth) / 2 : 0;
    const int startY = 1;

    // 테이블 헤더 선 그리기
    attron(A_BOLD | COLOR_PAIR(COLOR_PAIR_TEXT_SPEED));
    mvprintw(startY, startX, "╔═══════════════════════════════════════════════════════════════════════════════════════╗");
    if (stageFilter == 0)
    {
        mvprintw(startY + 1, startX, "║                               ALL STAGES RANKING BOARD                                ║");
    }
    else
    {
        mvprintw(startY + 1, startX, "║                                STAGE %d RANKING BOARD                                  ║", stageFilter);
    }
    mvprintw(startY + 2, startX, "╠═══════════════════════════════════════════════════════════════════════════════════════╣");
    attroff(A_BOLD | COLOR_PAIR(COLOR_PAIR_TEXT_SPEED));

    attron(A_BOLD);
    mvprintw(startY + 3, startX, "║ 순위 │  단계  │ 최대길이 │ Growth │ Poison │ Speed │ Gate통과 │      플레이 일시      ║");
    mvprintw(startY + 4, startX, "╠══════╪════════╪══════════╪════════╪════════╪═══════╪══════════╪═══════════════════════╣");
    attroff(A_BOLD);

    for (int i = 0; i < 11; i++)
    {
        const int rowY = startY + 5 + i;
        if (i < (int)ranks.size())
        {
            const auto &r = ranks[i];

            if (r.isCurrentPlay)
            {
                attron(COLOR_PAIR(COLOR_PAIR_TEXT_GROWTH) | A_BOLD);
                if (r.rank <= 10)
                {
                    mvprintw(rowY, startX, "║->%2d위│  %d단계 │    %2d    │   %2d   │   %2d   │  %2d   │    %2d    │  %s  ║",
                             r.rank, r.stage, r.maxLength, r.growthCount, r.poisonCount, r.speedCount, r.gateCount, r.timestamp.c_str());
                }
                else
                {
                    mvprintw(rowY, startX, "║내기록│  %d단계 │    %2d    │   %2d   │   %2d   │  %2d   │    %2d    │  %s  ║",
                             r.stage, r.maxLength, r.growthCount, r.poisonCount, r.speedCount, r.gateCount, r.timestamp.c_str());
                }
                attroff(COLOR_PAIR(COLOR_PAIR_TEXT_GROWTH) | A_BOLD);
            }
            else
            {
                if (r.rank == 1)
                {
                    attron(COLOR_PAIR(COLOR_PAIR_TEXT_USED_GATE) | A_BOLD);
                }
                else if (r.rank == 2)
                {
                    attron(COLOR_PAIR(COLOR_PAIR_TEXT_SPEED) | A_BOLD);
                }
                else if (r.rank == 3)
                {
                    attron(COLOR_PAIR(COLOR_PAIR_TEXT_GATE) | A_BOLD);
                }

                mvprintw(rowY, startX, "║  %2d위│  %d단계 │    %2d    │   %2d   │   %2d   │  %2d   │    %2d    │  %s  ║",
                         r.rank, r.stage, r.maxLength, r.growthCount, r.poisonCount, r.speedCount, r.gateCount, r.timestamp.c_str());

                if (r.rank <= 3)
                {
                    attroff(A_BOLD | COLOR_PAIR(COLOR_PAIR_TEXT_USED_GATE) | COLOR_PAIR(COLOR_PAIR_TEXT_SPEED) | COLOR_PAIR(COLOR_PAIR_TEXT_GATE));
                }
            }
        }
        else
        {
            mvprintw(rowY, startX, "║      │        │          │        │        │       │          │                       ║");
        }
    }

    mvprintw(startY + 16, startX, "╚═══════════════════════════════════════════════════════════════════════════════════════╝");

    attron(A_BOLD);
    const int visualWidth = getUtf8VisualWidth(bottomMessage);
    const int textX = std::max(startX, startX + (boxWidth - visualWidth) / 2);
    mvprintw(startY + 18, textX, "%s", bottomMessage.c_str());
    attroff(A_BOLD);

    refresh();
}

// 게임오버 시 성적을 기록 및 영속 저장하고 랭킹 보드 결과 스크린을 띄움 (다시시작 A는 true, 게임종료 Q는 false 반환)
bool GameController::showGameOverScreen(RankingManager &rankingManager) const
{
    // 1. 플레이 데이터 레코드 생성 및 하이라이트 플래그 지정 후 파일 저장
    rankingManager.addRecord(stageNum, scoreBoard.getMaxLength(), scoreBoard.getGrowthCount(), scoreBoard.getPoisonCount(), scoreBoard.getSpeedCount(), scoreBoard.getGateCount());
    rankingManager.markLatestAsCurrent();
    rankingManager.saveToFile();

    // 2. 랭킹 보드 띄우기
    const bool playAgain = showRankingBoardScreen(rankingManager, stageNum, "GAME OVER! [ A ] 처음부터 재시작 | [ Q ] 종료 | [← / →] 다른 랭킹", true);
    rankingManager.clearCurrentPlayFlag(); // 하이라이트 복원
    return playAgain;
}

// 스테이지 미션 성공 클리어 시 성적을 임시 기록 및 영속 저장하고 랭킹 결과 스크린을 띄움 (다음단계 A는 true, 게임종료 Q는 false 반환)
bool GameController::showStageClearScreen(RankingManager &rankingManager) const
{
    rankingManager.addRecord(stageNum, scoreBoard.getMaxLength(), scoreBoard.getGrowthCount(), scoreBoard.getPoisonCount(), scoreBoard.getSpeedCount(), scoreBoard.getGateCount());
    rankingManager.markLatestAsCurrent();
    rankingManager.saveToFile();

    const bool keepGoing = showRankingBoardScreen(rankingManager, stageNum, "STAGE CLEAR! [ A ] 다음 단계 계속 | [ Q ] 종료 | [← / →] 다른 랭킹", true);
    rankingManager.clearCurrentPlayFlag();
    return keepGoing;
}

// 전체 스테이지 완전 정복 성공 시 최종 누적 성적을 기록하고 엔딩 크레딧용 랭킹 스크린을 띄움
bool GameController::showTotalClearScreen(RankingManager &rankingManager, const int maxLength, const int growth, const int poison, const int speed, const int gate)
{
    rankingManager.addRecord(5, maxLength, growth, poison, speed, gate);
    rankingManager.markLatestAsCurrent();
    rankingManager.saveToFile();

    const bool playAgain = showRankingBoardScreen(rankingManager, 0, "ALL STAGES CLEAR! [ A ] 처음부터 재시작 | [ Q ] 종료 | [← / →] 다른 랭킹", true);
    rankingManager.clearCurrentPlayFlag();
    return playAgain;
}
