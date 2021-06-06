#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

typedef struct {
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

typedef enum { STATMENET_INSERT, STATEMENT_SELECT } StatementType;

typedef enum { PREPARE_SUCC, PREPARE_UNRECOGNIZED_STMT, PREPARE_SYNTAX_ERROR } PrepareStatement;

typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row;

typedef struct {
    StatementType type;
    Row row; // Only needed for insert
} Statement;

typedef enum { META_CMD_SUCC, META_CMD_UNRECOGNIZED } MetaCmdResult;

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;

// Does the row need to be a ptr??
void serialize_row(Row *source, void *destination) {
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void *source, Row *destination) {
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

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
        int args_assigned = sscanf(buf->buffer, "insert %d %s %s", &(stmt->row.id), stmt->row.username, stmt->row.email);

        if (args_assigned < 3)
            return PREPARE_SYNTAX_ERROR;
  
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
