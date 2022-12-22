#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "compiler.h"
#include "scanner.h"


typedef struct {
    Token current;
    Token previous;
    bool hiterror;
    bool panicking;
} Parser;

Parser parser;
Nugget* compiling_nugget;

static Nugget* current_nugget() {
    return compiling_nugget;
}

static void error_at(Token* token, const char* message) {
    if (parser.panicking) {
        return;
    }

    parser.panicking = true;

    fprintf(stderr, "[line %d] Error:", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end of input.");
    } else if (token->type == TOKEN_ERROR) {
        DO_NOTHING
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hiterror = true;
}


static void error_current_token(const char* message) {
    error_at(&parser.current, message);
}


static void error(const char* message) {
    error_at(&parser.previous, message);
}

/*
 * For the moment, just read one token at a time using scan_token() and print it.
 * A cool feature - the printf specified '%.*s' prints the first token.length characters starting at token.start.
 * This specifier therefore lets you pass the precision to which you want to print (in this case, the number of
 * characters) to printf as a separate argument. We need this here because a token .start pointer is pointing
 * somewhere into the original source string, so there is no NUL terminator at the end of the corresponding substring.
 * The * in the format specifier lets us limit this using an argument.
 */
bool compile(Nugget* nugget, const char* source) {
    init_scanner(source);
    parser.hiterror = false;
    parser.panicking = false;
    advance();
    expression();
    consume(TOKEN_EOF, "Expected end of expression!");
}


static void advance() {
    parser.previous = parser.current;

    for LOOP {
        parser.current = next_token();

        if (parser.current.type != TOKEN_ERROR) {
            break;
        }

        error_current_token(parser.current.start);
    }
}


static void consume(TokenType type, const char* message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    error_current_token(message);
}


static void emit_byte(uint8_t byte) {
    write_nugget(current_nugget(), byte, parser.previous.line);
}


bool compile_debug(Nugget* nugget, const char* source) {
    init_scanner(source);
    int line = -1;

    LOOP {
        Token token = scan_token();
        if (token.line != line) {
            printf("%4d", token.line);
            line = token.line;
        } else {
            printf("    |> ");
        }
        
        printf("%2d '%.*s'\n", token.type, token.length, token.start);

        if (token.type == TOKEN_EOF) {
            break;
        }
    }

    return !parser.hiterror;
}