#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

typedef enum { STATMENET_INSERT, STATEMENT_SELECT } StatementType;

typedef enum { PREPARE_SUCC, PREPARE_UNRECOGNIZED_STMT } PrepareStatement;

typedef struct {
    StatementType type;
} Statement;

typedef enum { META_CMD_SUCC, META_CMD_UNRECOGNIZED } MetaCmdResult;

InputBuffer* new_input_buffer() {
    InputBuffer *buf = (InputBuffer*)malloc(sizeof(InputBuffer));
    buf->buffer = NULL;
    buf->buffer_length = 0;
    buf->input_length = 0;
    return buf;
}

void print_prompt() { printf("sqlite> "); }

void close_input_buffer(InputBuffer *buf) {
    free(buf->buffer);
    free(buf);
}

void read_input(InputBuffer *buf) {
    ssize_t bytes_read = getline(&(buf->buffer), &(buf->buffer_length), stdin);

    if (bytes_read == 0) {
        printf("Bad input i guess :(\n");
        exit(EXIT_FAILURE);
    }

    // Ignore \n at the end
    buf->input_length = bytes_read - 1;
    buf->buffer[bytes_read - 1] = '\0'; // Terminate one position earlier
}

PrepareStatement prepare_stmt(InputBuffer *buf, Statement *stmt) {
    if (strncmp(buf->buffer, "insert", 6) == 0) {
        stmt->type = STATMENET_INSERT;
        return PREPARE_SUCC;
    }
    if (strncmp(buf->buffer, "select", 6) == 0) {
        stmt->type = STATEMENT_SELECT;
        return PREPARE_SUCC;
    }

    return PREPARE_UNRECOGNIZED_STMT;
}

MetaCmdResult do_meta_command(InputBuffer *buf) {
    if (strcmp(buf->buffer, ".exit") == 0) {
        exit(EXIT_SUCCESS);
    } else {
        return META_CMD_UNRECOGNIZED;
    }
}

void execute_statement(Statement stmt) {
    switch (stmt.type) {
        case (STATEMENT_SELECT):
            //select
            printf("Selecting... lol");
            break;
        case (STATMENET_INSERT):
            //insert
            printf("Inserting... lol");
            break; 
    }
}

int main(int argc, char *argv[]) {
    InputBuffer *buf = new_input_buffer();
    while (true) {
        print_prompt();
        read_input(buf);

        if (buf->buffer[0] == '.') {
            switch (do_meta_command(buf)) {
                case (META_CMD_SUCC):
                    continue;
                case (META_CMD_UNRECOGNIZED):
                    printf("Unrecognized command '%s'\n", buf->buffer);
                    continue;
            }
        }

        Statement stmt;
        switch (prepare_stmt(buf, &stmt)) {
            case (PREPARE_SUCC):
                break;
            case (PREPARE_UNRECOGNIZED_STMT):
                printf("Unrecognized verb at the start of '%s'\n", buf->buffer);
        }

        execute_statement(stmt);
        printf("Executed.\n");
    }
}
