
#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LEN 50

    typedef struct LeaderboardNode {
        char name[MAX_NAME_LEN];
        int score;
        struct LeaderboardNode* next;
    } LeaderboardNode;

    /* Function prototypes implemented in Leaderboard.c */
    LeaderboardNode* createLeaderboardNode(const char* name, int score);
    void insertLeaderboard(LeaderboardNode** head, LeaderboardNode* newNode);
    void insertLeaderboardInOrder(LeaderboardNode** head, LeaderboardNode* newNode); // insert descending
    void sortLeaderboard(LeaderboardNode** head);
    void trimLeaderboard(LeaderboardNode** head);
    void showLeaderboard(LeaderboardNode* head);
    void freeLeaderboardList(LeaderboardNode* head);
    int leaderboardCount(LeaderboardNode* head); // returns number of entries

#ifdef __cplusplus
}
#endif

#endif // LEADERBOARD_H