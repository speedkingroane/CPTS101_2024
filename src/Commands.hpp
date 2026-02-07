#ifndef COMMANDS_H
#define COMMANDS_H
#define _CRT_SECURE_NO_WARNINGS 
#include <stdio.h>

#define MAX_CMD_LEN 50
#define MAX_DESC_LEN 200

typedef struct CommandNode {
    char command[MAX_CMD_LEN];
    char description[MAX_DESC_LEN];
    int points;
    struct CommandNode* next;
} CommandNode;

// Node creation
CommandNode* createCommandNode(const char* cmd, const char* desc, int pts);

// Linked list operations
void insertCommandEnd(CommandNode** head, CommandNode* newNode);
CommandNode* searchCommand(CommandNode* head, const char* cmd);
void removeCommandNode(CommandNode** head, const char* cmd);
void displayCommands(CommandNode* head);

// Memory management
void freeCommandList(CommandNode* head);

// File I/O
void loadCommands(CommandNode** head);
void saveCommands(CommandNode* head);

#endif // COMMANDS_H
