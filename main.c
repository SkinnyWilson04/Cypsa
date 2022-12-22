#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "compiler.h"
#include "nugget.h"
#include "debug.h"
#include "vm.h"


/*
 * A slightly creaky repl() implementation:
 *    - The line size is card-coded. fgets() means this is safe, but restrictive.
 *    - Input which spans over multiple lines is not handled (yet).
 */
static void repl() {
    char repl_line[2048];

    LOOP {
        printf(">>> ");

        if (!fgets(repl_line, sizeof(repl_line), stdin)) {
            printf("\n");
            break;
        }

        interpret(repl_line);
    }
}


/*
 * Read a text file from filepath.
 * Seek to end and call ftell() to get the difference, in bytes, between the start of the file and the current pointer (so, just the file
 * size in this case), then rewind pointer back to start. Read the file contents into a buffer and return it.
 * Note that the function returns a malloc'd pointer so it is the responsibility of the calling function to free it.
 *     TODO:
 *         -> A bunch of this process can fail but retyping the error reporting is a pain in the arse. Make this more generic.
 *              (Maybe have some enums which indicate common types of errors than can be switched on?)
 */
static char* read_file(const char* filepath) {
    FILE* file = fopen(filepath, "rb");

    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file at location '%s'.\nCheck path and retry.\n", filepath);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t filesize_bytes = ftell(file);
    rewind(file);
    
    char* input_buffer = malloc(filesize_bytes + 1);

    if (input_buffer == NULL) {
        fprintf(stderr, "OUT OF MEMORY Error: Could not allocate memory of size %I64u bytes.", filesize_bytes);
        exit(74);
    }

    size_t bytes_read = fread(input_buffer, sizeof(char), filesize_bytes, file);
    
    if (input_buffer == NULL) {
        fprintf(stderr, "Error: Could not read entire file '%s' - managed to read [%I64u] of [%I64u] total bytes.", filepath, bytes_read, filesize_bytes);
    }

    input_buffer[bytes_read] = '\0';
    fclose(file);
    return input_buffer;
}


/*
 * Read source code from file and interpret it.
 * Interpret returns a status enum which indicates how the program terminated. Exit with a unique exit code for each.
 */
static void run_from_file(const char* filepath) {
    char* source = read_file(filepath);
    InterpretationResult result = interpret(source);
    free(source);

    if (result == INTERPRETER_COMPILE_ERROR) {
        exit(65);
    }
    if (result == INTERPRETER_RUNTIME_ERROR) {
        exit(70);
    }
}



/*
 * GO
 */
int main(int argc, char* argv[]) {
    init_VM();

    Nugget nugget;
    init_nugget(&nugget);
    
    Value v1 = 1.0;
    Value v2 = 2.0;
    Value v3 = 3.0;

    write_constant(&nugget, v1, 1);
    write_constant(&nugget, v2, 2);

    write_constant(&nugget, v3, 3);
    write_nugget(&nugget, OPCODE_NEGATE, 4); 

    write_constant(&nugget, 123.456789, 3);
    write_constant(&nugget, 123.456789, 11);
    write_constant(&nugget, 123.456789, 12);
    write_constant(&nugget, 123.456789, 13);
    write_constant(&nugget, 123.456789, 14);
    write_constant(&nugget, 123.456789, 15);
    write_constant(&nugget, 123.456789, 16);
    write_constant(&nugget, 123.456789, 17);
    write_constant(&nugget, 123.456789, 18);

    write_nugget(&nugget, OPCODE_MULTIPLY, 10);

    write_nugget(&nugget, OPCODE_RETURN, 20);

    if (argc > 1) {
        printf("\nRunning from file: %s\n", argv[1]);
        run_from_file(argv[1]);
    } else {
        printf("\nEntering REPL...\n\n");
        repl();
    }

    free_nugget(&nugget);
    free_VM();

    return EXIT_SUCCESS;
}