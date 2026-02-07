#include "Leaderboard.hpp"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

LeaderboardNode* createLeaderboardNode(const char* name, int score) {
    LeaderboardNode* node = malloc(sizeof(LeaderboardNode));
    if (!node) {
        printf("Memory allocation failed.\n");
        return NULL;
    }

    strncpy(node->name, name, MAX_NAME_LEN - 1);
    node->name[MAX_NAME_LEN - 1] = '\0';
    node->score = score;
    node->next = NULL;

    return node;
}

void insertLeaderboard(LeaderboardNode** head, LeaderboardNode* newNode) {
    if (*head == NULL) {
        *head = newNode;
        return;
    }

    LeaderboardNode* curr = *head;
    while (curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = newNode;
}

/* Insert newNode into the list in descending score order.
   If multiple nodes have the same score, newNode is placed after them.
   After insert, trim to top 3 (so list remains at most 3 entries). */
void insertLeaderboardInOrder(LeaderboardNode** head, LeaderboardNode* newNode) {
    if (newNode == NULL) return;

    if (*head == NULL || newNode->score > (*head)->score) {
        newNode->next = *head;
        *head = newNode;
        trimLeaderboard(head);
        return;
    }

    LeaderboardNode* curr = *head;
    while (curr->next != NULL && curr->next->score >= newNode->score) {
        curr = curr->next;
    }

    newNode->next = curr->next;
    curr->next = newNode;

    trimLeaderboard(head);
}

int leaderboardCount(LeaderboardNode* head) {
    int c = 0;
    while (head) { c++; head = head->next; }
    return c;
}

void sortLeaderboard(LeaderboardNode** head) {
    if (*head == NULL || (*head)->next == NULL) return;

    int swapped;
    LeaderboardNode* ptr1;
    LeaderboardNode* lptr = NULL;

    do {
        swapped = 0;
        ptr1 = *head;

        while (ptr1->next != lptr) {
            if (ptr1->score < ptr1->next->score) {
                // swap data
                int tempScore = ptr1->score;
                char tempName[MAX_NAME_LEN];

                strcpy(tempName, ptr1->name);
                ptr1->score = ptr1->next->score;
                strcpy(ptr1->name, ptr1->next->name);

                ptr1->next->score = tempScore;
                strcpy(ptr1->next->name, tempName);

                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    } while (swapped);
}


void trimLeaderboard(LeaderboardNode** head) {
    if (*head == NULL) return;

    int count = 1;
    LeaderboardNode* curr = *head;

    while (curr && curr->next) {
        if (count == 3) {
            // delete everything after curr
            LeaderboardNode* toDelete = curr->next;
            curr->next = NULL;

            while (toDelete) {
                LeaderboardNode* temp = toDelete;
                toDelete = toDelete->next;
                free(temp);
            }
            return;
        }
        curr = curr->next;
        count++;
    }
}


void showLeaderboard(LeaderboardNode* head) {
    if (head == NULL) {
        printf("Leaderboard is empty.\n");
        return;
    }

    printf("\n==== Leaderboard (Top 3) ====\n");
    int rank = 1;
    while (head != NULL && rank <= 3) {
        printf("%d. %s - %d\n", rank, head->name, head->score);
        head = head->next;
        rank++;
    }
}


void freeLeaderboardList(LeaderboardNode* head) {
    while (head != NULL) {
        LeaderboardNode* temp = head;
        head = head->next;
        free(temp);
    }
}