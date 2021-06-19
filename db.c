#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define TABLE_MAX_PAGES 100

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

typedef struct {
    uint32_t num_rows;
    void *pages[TABLE_MAX_PAGES];
} Table;

typedef enum { META_CMD_SUCC, META_CMD_UNRECOGNIZED } MetaCmdResult;

typedef enum { EXECUTE_SUCCESS, EXECUTE_TABLE_FULL } ExecuteResult;

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ROW_SIZE = ID_SIZE + EMAIL_SIZE + USERNAME_SIZE;
const uint32_t PAGE_SIZE = 4096; // Same size of virtual memory page on most architectures
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;

void print_row(Row *row) {
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

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

Table* new_table() {
    Table* table = (Table *)malloc(sizeof(Table));
    table->num_rows = 0;
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        table->pages[i] = NULL;
    }
    return table;
}

void free_table(Table *table) {
    for (uint32_t i = 0; table->pages[i]; i++) {
        free(table->pages[i]);
    }
    free(table);
}

/* Returns a pointer to the start of the serialized data of row # row_num */
void* row_slot(Table *table, uint32_t row_num) {
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void *page = table->pages[page_num];
    if (page == NULL) {
        page = table->pages[page_num] = malloc(PAGE_SIZE);
    }
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return page + byte_offset;
}

InputBuffer* new_input_buffer() {
    InputBuffer *buf = (InputBuffer*)malloc(sizeof(InputBuffer));
    buf->buffer = NULL;
    buf->buffer_length = 0;
    buf->input_length = 0;
    return buf;
}

void print_prompt() { printf("db > "); }

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

MetaCmdResult do_meta_command(InputBuffer *buf, Table *table) {
    if (strcmp(buf->buffer, ".exit") == 0) {
        close_input_buffer(buf);
        exit(EXIT_SUCCESS);
        free_table(table);
    } else {
        return META_CMD_UNRECOGNIZED;
    }
}

ExecuteResult execute_insert(Statement *stmt, Table *table) {
    if (table->num_rows >= TABLE_MAX_ROWS) {
        return EXECUTE_TABLE_FULL;
    }

    Row *row_to_insert = &(stmt->row);

    serialize_row(row_to_insert, row_slot(table, table->num_rows));
    table->num_rows += 1;

    return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement *stmt, Table *table) {
    Row row;
    for (uint32_t i = 0; i < table->num_rows; i++) {
        deserialize_row(row_slot(table, i), &row);
        print_row(&row);
    }

    return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement *stmt, Table *table) {
    switch (stmt->type) {
        case (STATEMENT_SELECT):
            return execute_select(stmt, table);
        case (STATMENET_INSERT):
            return execute_insert(stmt, table);
            break; 
    }
}

int main(int argc, char *argv[]) {
    Table *table = new_table();
    InputBuffer *buf = new_input_buffer();
    while (true) {
        print_prompt();
        read_input(buf);

        if (buf->buffer[0] == '.') {
            switch (do_meta_command(buf, table)) {
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
                continue;
            case (PREPARE_SYNTAX_ERROR):
                printf("Syntax error. Could not parse the statement.\n");
                continue;
        }

        switch(execute_statement(&stmt, table)) {
            case EXECUTE_SUCCESS:
                printf("Executed.\n");
                break;
            case EXECUTE_TABLE_FULL:
                printf("Error: Table full.\n");
                break;
        }
    }
    free_table(table);
}
