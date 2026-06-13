// RankingManager.h
// 플레이 결과를 파일(scoreboard/rankings.txt)에 세이브 및 로드하여 영속화
// 성적순(최대길이 -> 도달단계 -> Growth -> Poison 등) 다차원 정렬 수행
// 스테이지별 상위 10등 목록 및 방금 플레이한 성적 하이라이트 제공

#ifndef RANKINGMANAGER_H
#define RANKINGMANAGER_H

#include <string>
#include <vector>

// 단일 플레이어 랭킹 레코드 구조체
struct RankingRecord
{
    std::string timestamp;      // 기록 생성 시점의 날짜 및 시간 문자열
    int stage;                  // 도달하거나 클리어한 스테이지
    int maxLength;              // 게임 중 달성한 최장 뱀 길이
    int growthCount;            // 획득한 성장 아이템의 총 누적 개수
    int poisonCount;            // 획득한 독약 아이템의 총 누적 개수
    int speedCount;             // 획득한 스피드 아이템의 총 누적 개수
    int gateCount;              // 통과한 게이트의 총 누적 횟수
    bool isCurrentPlay = false; // 현재 판(방금 끝난 판)의 기록인지 여부 (하이라이트 표시용)
    int rank = 0;               // 정렬 후 부여된 등수 (1부터 시작)
};

// 파일 입출력을 처리하고 점수판 기록 정렬 및 관리를 담당하는 클래스
class RankingManager
{
public:
    RankingManager();
    ~RankingManager() = default;

    // 파일에서 랭킹 로드
    void loadFromFile(const std::string &filepath = "scoreboard/rankings.txt");

    // 파일에 랭킹 저장
    void saveToFile(const std::string &filepath = "scoreboard/rankings.txt") const;

    // 새로운 랭킹 레코드 추가
    // 추가 시 현재 시간 구해서 timestamp 자동 생성
    void addRecord(const int stage, const int maxLength, const int growth, const int poison, const int speed, const int gate);

    // 특정 스테이지의 랭킹 리스트 반환
    // 최대 길이 내림차순 정렬 후 상위 10등 + 방금 플레이한 기록이 10등 밖일 시 추가 반환
    std::vector<RankingRecord> getRankings(const int stageFilter) const;

    // 방금 플레이한 기록에 플레이 마크(isCurrentPlay)를 표시
    void markLatestAsCurrent();

    // 방금 플레이한 기록 표시 해제
    void clearCurrentPlayFlag();

private:
    std::vector<RankingRecord> records; // 등록된 모든 랭킹 레코드 목록 배열
};

#endif
