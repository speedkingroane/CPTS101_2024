#include "Commands.hpp"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int parse_csv_fields(const char* line,
                            char* out_cmd, size_t cmd_len,
                            char* out_desc, size_t desc_len,
                            char* out_pts, size_t pts_len)
{
    const char* p = line;
    char* dest = NULL;
    size_t dest_len = 0;
    int field = 0;

    // initialize outputs
    if (out_cmd && cmd_len) out_cmd[0] = '\0';
    if (out_desc && desc_len) out_desc[0] = '\0';
    if (out_pts && pts_len) out_pts[0] = '\0';

    while (*p != '\0' && *p != '\r' && *p != '\n' && field < 3) {
        // select destination buffer
        if (field == 0) { dest = out_cmd; dest_len = cmd_len; }
        else if (field == 1) { dest = out_desc; dest_len = desc_len; }
        else { dest = out_pts; dest_len = pts_len; }

        size_t i = 0;
        // skip leading spaces
        while (*p == ' ' || *p == '\t') p++;

        if (*p == '"') {
            // quoted field
            p++; // skip opening quote
            while (*p != '\0') {
                if (*p == '"') {
                    if (p[1] == '"') {
                        // escaped quote -> append one quote
                        if (i + 1 < dest_len) dest[i++] = '"';
                        p += 2;
                        continue;
                    } else {
                        // closing quote
                        p++;
                        break;
                    }
                }
                if (i + 1 < dest_len) dest[i++] = *p;
                p++;
            }
        } else {
            // unquoted field
            while (*p != '\0' && *p != ',' && *p != '\r' && *p != '\n') {
                if (i + 1 < dest_len) dest[i++] = *p;
                p++;
            }
        }

        // trim trailing spaces in destination
        while (i > 0 && (dest[i - 1] == ' ' || dest[i - 1] == '\t')) i--;
        if (dest_len) dest[i < dest_len ? i : dest_len - 1] = '\0';

        // skip optional spaces after field
        while (*p == ' ' || *p == '\t') p++;

        // if current char is comma, skip it and continue to next field
        if (*p == ',') p++;

        field++;
    }

    // success if we parsed all three fields
    return (field == 3) ? 1 : 0;
}

static char* escape_csv_field(const char* src)
{
    int needs_quotes = 0;
    size_t len = 0;
    const char* p = src;
    while (*p) {
        if (*p == '"' || *p == ',' || *p == '\n' || *p == '\r') needs_quotes = 1;
        if (*p == '"') len += 2; // will be doubled
        else len += 1;
        p++;
    }

    if (!needs_quotes) {
        // no special chars, return a copy
        char* out = (char*)malloc(len + 1);
        if (!out) return NULL;
        strcpy(out, src);
        return out;
    }

    // need quotes and doubling quotes
    // allocate len + 2 for surrounding quotes + 1 for null
    char* out = (char*)malloc(len + 3);
    if (!out) return NULL;
    char* d = out;
    *d++ = '"';
    p = src;
    while (*p) {
        if (*p == '"') {
            *d++ = '"';
            *d++ = '"';
        } else {
            *d++ = *p;
        }
        p++;
    }
    *d++ = '"';
    *d = '\0';
    return out;
}

void loadCommands(CommandNode** head) {
    FILE* file = fopen("Commands.csv", "r");
    char line[512];

    if (file == NULL) {
        printf("Commands.csv not found. Starting with empty command list.\n");
        return;
    }

    // Read header (if present) and ignore
    if (fgets(line, sizeof(line), file) == NULL) {
        fclose(file);
        return;
    }

    while (fgets(line, sizeof(line), file)) {
        // remove newline(s)
        line[strcspn(line, "\r\n")] = '\0';

        char cmd[MAX_CMD_LEN];
        char desc[MAX_DESC_LEN];
        char ptsStr[32];

        if (!parse_csv_fields(line, cmd, sizeof(cmd), desc, sizeof(desc), ptsStr, sizeof(ptsStr))) {
            // malformed line
            continue;
        }

        // trim leading/trailing whitespace (already trimmed trailing in parser; trim leading now)
        // leading spaces handled by parser already

        int points = atoi(ptsStr);

        // Prevent duplicates
        if (searchCommand(*head, cmd) != NULL) {
            continue;
        }

        CommandNode* node = createCommandNode(cmd, desc, points);
        if (node != NULL) {
            insertCommandEnd(head, node);
        }
    }

    fclose(file);
}

void saveCommands(CommandNode* head) {
    const char* tmpName = "Commands.tmp";
    const char* outName = "Commands.csv";

    FILE* file = fopen(tmpName, "w");
    if (file == NULL) {
        printf("Failed to open temporary file for saving commands.\n");
        return;
    }

    // Write header
    if (fprintf(file, "command,description,points\n") < 0) {
        printf("Failed to write header to temporary file.\n");
        fclose(file);
        remove(tmpName);
        return;
    }

    while (head != NULL) {
        char* esc_cmd = escape_csv_field(head->command);
        char* esc_desc = escape_csv_field(head->description);
        if (esc_cmd == NULL || esc_desc == NULL) {
            free(esc_cmd);
            free(esc_desc);
            fclose(file);
            remove(tmpName);
            printf("Memory allocation failed while saving commands.\n");
            return;
        }

        if (fprintf(file, "%s,%s,%d\n", esc_cmd, esc_desc, head->points) < 0) {
            printf("Failed to write command to temporary file.\n");
            free(esc_cmd);
            free(esc_desc);
            fclose(file);
            remove(tmpName);
            return;
        }

        free(esc_cmd);
        free(esc_desc);
        head = head->next;
    }

    fflush(file);
    fclose(file);

    // Replace the old file. On Windows rename will fail if target exists, so remove first.
    remove(outName);
    if (rename(tmpName, outName) != 0) {
        // fallback: try copying if rename fails (rare)
        printf("Failed to rename temporary file to %s\n", outName);
        remove(tmpName);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------


CommandNode* createCommandNode(const char* cmd, const char* desc, int pts) {
    CommandNode* newNode = (CommandNode*)malloc(sizeof(CommandNode));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        return NULL;
    }

    // Copy data safely
    strncpy(newNode->command, cmd, MAX_CMD_LEN - 1);
    newNode->command[MAX_CMD_LEN - 1] = '\0';

    strncpy(newNode->description, desc, MAX_DESC_LEN - 1);
    newNode->description[MAX_DESC_LEN - 1] = '\0';

    newNode->points = pts;
    newNode->next = NULL;

    return newNode;
}

CommandNode* searchCommand(CommandNode* head, const char* cmd)
{
    while (head != NULL) {
        if (strcmp(head->command, cmd) == 0) {
            return head;  // Found
        }
        head = head->next;
    }
    return NULL;  // Not found
}

void insertCommandEnd(CommandNode** head, CommandNode* newNode) {
    if (*head == NULL) {
        *head = newNode;
        return;
    }

    CommandNode* current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newNode;
}

void removeCommandNode(CommandNode** head, const char* cmd)
{
    if (*head == NULL) {
        printf("Command list is empty.\n");
        return;
    }

    CommandNode* current = *head;
    CommandNode* prev = NULL;

    while (current != NULL && strcmp(current->command, cmd) != 0) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        printf("Command '%s' not found.\n", cmd);
        return;
    }

    // Removing head node
    if (prev == NULL) {
        *head = current->next;
    }
    else {
        prev->next = current->next;
    }

    free(current);
    printf("Command '%s' removed successfully.\n", cmd);
}

void displayCommands(CommandNode* head)
{
    if (head == NULL) {
        printf("No commands available.\n");
        return;
    }

    printf("\n%-15s | %-60s | Points\n", "Command", "Description");
    printf("--------------------------------------------------------------------------\n");

    while (head != NULL) {
        printf("%-15s | %-60s | %d\n",
            head->command,
            head->description,
            head->points);
        head = head->next;
    }
}

void freeCommandList(CommandNode* head) {
    CommandNode* current = head;
    while (current != NULL) {
        CommandNode* next = current->next;
        free(current);
        current = next;
    }
}

