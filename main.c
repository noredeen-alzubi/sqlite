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

int main(int argc, char *argv[]) {
    InputBuffer *buf = new_input_buffer();
    while (true) {
        print_prompt();
        read_input(buf);

        if (strcmp(buf->buffer, "\\q") == 0) {
            close_input_buffer(buf);
            exit(EXIT_SUCCESS);
        } else {
            printf("Voodoo command '%s'.\n", buf->buffer);
        }
    }
}
