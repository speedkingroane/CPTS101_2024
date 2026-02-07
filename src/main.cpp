//Spencer Roane
//Professor Subu
//CPT_S 223
//1/29/2026 - 2/6/2026
//This program is a command-line quiz game that tests users on 
//their knowledge of various commands.
//-----------------------------------------------------------
//Reflection:
//One pro to using a linked list for commands is that we can easily add or remove commands without resizing an array or shifting any elements.
//One con is that the time to find a command  is linear (O(n)) 
//due to the need to traverse the list one by one, making the process of finding 
//a random question both longer and harder
//--------------------------------------------------------------------------------------------------------------------
#define  _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Commands.hpp"
#include "Leaderboard.hpp"

#define QUESTIONS_PER_GAME 20
#define LEADERBOARD_FILE "Leaderboard.csv"

/* Local helpers for leaderboard persistence (simple CSV read/write) */
static void loadLeaderboardFromFile(LeaderboardNode** head) {
    FILE* f = fopen(LEADERBOARD_FILE, "r");
    if (!f) return;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = '\0';
        char* comma = strrchr(line, ',');
        if (!comma) continue;
        *comma = '\0';
        const char* name = line;
        int score = atoi(comma + 1);
        insertLeaderboard(head, createLeaderboardNode(name, score));
    }
    fclose(f);
    sortLeaderboard(head);
    trimLeaderboard(head);
}

static void saveLeaderboardToFile(LeaderboardNode* head) {
    FILE* f = fopen(LEADERBOARD_FILE, "w");
    if (!f) return;
    while (head) {
        fprintf(f, "%s,%d\n", head->name, head->score);
        head = head->next;
    }
    fclose(f);
}

/* Utility: read a line from stdin into buf, stripping newline */
static void readLine(char* buf, size_t len) {
    if (fgets(buf, (int)len, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    buf[strcspn(buf, "\r\n")] = '\0';
}

/* Menu functions */
static void viewRules(void) {
    printf("\n===== Rules =====\n");
    printf("You will be given up to %d multiple-choice questions.\n", QUESTIONS_PER_GAME);
    printf("Each question shows a description; choose the correct command name among 3 options.\n");
    printf("Correct answer awards the command's point value. Two wrong answers are provided.\n");
    printf("At the end your total points are shown. If it's a top score you can enter your name.\n\n");
}

static int countCommands(CommandNode* head) {
    int c = 0;
    while (head) { c++; head = head->next; }
    return c;
}


// ... (keep the existing includes and other functions above)

static int pickRandomCommands(CommandNode* head, CommandNode** out, int maxCount)
{
    if (head == NULL || out == NULL || maxCount <= 0) return 0;

    // Count and collect pointers
    int total = 0;
    for (CommandNode* p = head; p != NULL; p = p->next) total++;
    if (total == 0) return 0;

    // Allocate temporary array of pointers
    CommandNode** temp = (CommandNode**)malloc(sizeof(CommandNode*) * total);
    if (!temp) return 0;

    CommandNode* p = head;
    for (int i = 0; i < total; ++i) { temp[i] = p; p = p->next; }

    // Fisher–Yates shuffle
    for (int i = total - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        CommandNode* t = temp[i];
        temp[i] = temp[j];
        temp[j] = t;
    }

    // Copy up to maxCount into out
    int selected = (total < maxCount) ? total : maxCount;
    for (int i = 0; i < selected; ++i) out[i] = temp[i];

    free(temp);
    return selected;
}

/* Play a new game: select up to QUESTIONS_PER_GAME unique commands from the list */
static void playNewGame(CommandNode* commandsHead, LeaderboardNode** leaderboard) {
    int totalCmds = countCommands(commandsHead);
    if (totalCmds < 3) {
        printf("Not enough commands to play (need at least 3).\n");
        return;
    }

    int numQuestions = QUESTIONS_PER_GAME;
    if (totalCmds < QUESTIONS_PER_GAME) numQuestions = totalCmds;

    // Use helper to pick unique commands
    CommandNode** selected = (CommandNode**)malloc(sizeof(CommandNode*) * numQuestions);
    if (!selected) return;
    int picked = pickRandomCommands(commandsHead, selected, numQuestions);

    int score = 0;
    char input[64];

    // Build an array of all commands once for sampling wrong answers
    CommandNode** all = (CommandNode**)malloc(sizeof(CommandNode*) * totalCmds);
    if (!all) { free(selected); return; }
    CommandNode* curtmp = commandsHead;
    for (int i = 0; i < totalCmds; ++i) { all[i] = curtmp; curtmp = curtmp->next; }

    for (int q = 0; q < picked; ++q) {
        CommandNode* correct = selected[q];

        // pick two wrong unique indices (ensure different from q)
        int wrong1 = -1, wrong2 = -1;
        if (totalCmds <= 3) {
            // small list: pick two distinct wrong indices from the full pool (exclude correct)
            int correctIndexInAll = -1;
            for (int i = 0; i < totalCmds; ++i) if (all[i] == correct) { correctIndexInAll = i; break; }
            do { wrong1 = rand() % totalCmds; } while (wrong1 == correctIndexInAll);
            do { wrong2 = rand() % totalCmds; } while (wrong2 == correctIndexInAll || wrong2 == wrong1);
            const char* opts[3] = { correct->command, all[wrong1]->command, all[wrong2]->command };

            // shuffle and continue
            for (int i = 2; i > 0; --i) {
                int j = rand() % (i + 1);
                const char* t = opts[i]; opts[i] = opts[j]; opts[j] = t;
            }
            int correctChoice = -1;
            for (int i = 0; i < 3; ++i) if (strcmp(opts[i], correct->command) == 0) correctChoice = i + 1;

            printf("\nQuestion %d of %d\n", q + 1, picked);
            printf("%s\n", correct->description);
            for (int i = 0; i < 3; ++i) printf("%d) %s\n", i + 1, opts[i]);

            int choice = 0;
            while (1) {
                printf("Your answer (1-3): ");
                readLine(input, sizeof(input));
                if (sscanf(input, "%d", &choice) == 1 && choice >= 1 && choice <= 3) break;
                printf("Invalid selection. Try again.\n");
            }

            if (choice == correctChoice) {
                printf("Correct! +%d points.\n", correct->points);
                score += correct->points;
            }
            else {
                printf("Wrong. Correct answer was: %s\n", correct->command);
            }
        }
        else {
            // General path: sample wrong answers from the full pool (exclude correct)
            int correctIndexInAll = -1;
            for (int i = 0; i < totalCmds; ++i) if (all[i] == correct) { correctIndexInAll = i; break; }
            int r1, r2;
            do { r1 = rand() % totalCmds; } while (r1 == correctIndexInAll);
            do { r2 = rand() % totalCmds; } while (r2 == correctIndexInAll || r2 == r1);
            const char* opts[3] = { correct->command, all[r1]->command, all[r2]->command };

            // shuffle options
            for (int i = 2; i > 0; --i) {
                int j = rand() % (i + 1);
                const char* t = opts[i]; opts[i] = opts[j]; opts[j] = t;
            }

            int correctChoice = -1;
            for (int i = 0; i < 3; ++i) if (strcmp(opts[i], correct->command) == 0) correctChoice = i + 1;

            printf("\nQuestion %d of %d\n", q + 1, picked);
            printf("%s\n", correct->description);
            for (int i = 0; i < 3; ++i) printf("%d) %s\n", i + 1, opts[i]);

            int choice = 0;
            while (1) {
                printf("Your answer (1-3): ");
                readLine(input, sizeof(input));
                if (sscanf(input, "%d", &choice) == 1 && choice >= 1 && choice <= 3) break;
                printf("Invalid selection. Try again.\n");
            }

            if (choice == correctChoice) {
                printf("Correct! +%d points.\n", correct->points);
                score += correct->points;
            }
            else {
                printf("Wrong. Correct answer was: %s\n", correct->command);
            }
        }
    }

    free(all);
    free(selected);
    // ... keep earlier includes and helpers above unchanged

 // Replace the end-of-game leaderboard check in playNewGame with this logic:
     // after printing final score:
    printf("\nGame over. Total score: %d\n", score);

    int count = leaderboardCount(*leaderboard);
    int qualifies = 0;

    if (count == 0) {
        qualifies = 1; // empty leaderboard -> ask for name and insert
    }
    else if (count < 3) {
        // fewer than 3 entries -> qualifies automatically
        qualifies = 1;
    }
    else {
        // find the 3rd place score
        LeaderboardNode* tmp = *leaderboard;
        int rank = 1;
        int thirdScore = tmp->score; // default
        while (tmp && rank <= 3) {
            if (rank == 3) { thirdScore = tmp->score; break; }
            tmp = tmp->next;
            rank++;
        }
        if (score > thirdScore) qualifies = 1;
    }

    if (qualifies) {
        char name[MAX_NAME_LEN];
        printf("Congratulations! You made the leaderboard. Enter your name: ");
        readLine(name, sizeof(name));
        if (name[0] == '\0') strcpy(name, "Anonymous");
        LeaderboardNode* newNode = createLeaderboardNode(name, score);
        if (newNode) {
            insertLeaderboardInOrder(leaderboard, newNode);
            saveLeaderboardToFile(*leaderboard);
            printf("Leaderboard updated.\n");
        }
    }
    else {
        printf("Score did not qualify for the top 3.\n");
    }

    // ... rest of function continues unchanged

}
//---------------------------------------------------------------
static void addCommandInteractive(CommandNode** head) {
    char cmd[MAX_CMD_LEN];
    char desc[MAX_DESC_LEN];
    char ptsStr[32];
    int pts = 0;

    printf("Enter command name: ");
    readLine(cmd, sizeof(cmd));
    if (cmd[0] == '\0') { printf("Empty command name. Cancelled.\n"); return; }

    printf("Enter description: ");
    readLine(desc, sizeof(desc));

    printf("Enter points (integer): ");
    readLine(ptsStr, sizeof(ptsStr));
    pts = atoi(ptsStr);

    if (searchCommand(*head, cmd) != NULL) {
        printf("Command '%s' already exists. Not added.\n", cmd);
        return;
    }

    CommandNode* node = createCommandNode(cmd, desc, pts);
    if (node) {
        insertCommandEnd(head, node);
        printf("Command '%s' added.\n", cmd);
    }
}

static void removeCommandInteractive(CommandNode** head) {
    char cmd[MAX_CMD_LEN];
    printf("Enter command name to remove: ");
    readLine(cmd, sizeof(cmd));
    if (cmd[0] == '\0') { printf("Empty command name. Cancelled.\n"); return; }
    removeCommandNode(head, cmd);
}
//-----------------------------------------------MAIN MENU---------------------------------------------------------------------
/* Main menu loop */
int main(void) {
    srand((unsigned)time(NULL));

    CommandNode* commands = NULL;
    LeaderboardNode* leaderboard = NULL;

    // load commands and leaderboard---------------------------------------------
    loadCommands(&commands);
    loadLeaderboardFromFile(&leaderboard);

    while (1) {
        printf("\n==== Main Menu ====\n");
        printf("1) View Rules\n");
        printf("2) Play a New Game\n");
        printf("3) Add a Command\n");
        printf("4) Remove a Command\n");
        printf("5) Display All Commands\n");
        printf("6) Show Leaderboard\n");
        printf("7) Save and Exit\n");
        printf("Choose an option (1-7): ");

        char input[32];
        readLine(input, sizeof(input));
        int opt = atoi(input);

        switch (opt) {
        case 1:
            viewRules();
            break;
        case 2:
            playNewGame(commands, &leaderboard);
            break;
        case 3:
            addCommandInteractive(&commands);
            break;
        case 4:
            removeCommandInteractive(&commands);
            break;
        case 5:
            displayCommands(commands);
            break;
        case 6:
            showLeaderboard(leaderboard);
            break;
        case 7:
            saveCommands(commands);
            saveLeaderboardToFile(leaderboard);
            freeCommandList(commands);
            freeLeaderboardList(leaderboard);
            printf("Saved. Exiting.\n");
            return 0;
        default:
            printf("Invalid selection. Try again.\n");
            break;
        }
    }
    return 0;
}