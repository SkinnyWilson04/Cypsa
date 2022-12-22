#include <stdio.h>
#include <string.h>
#include "common.h"
#include "scanner.h"

/*
 * The scanner scans through the source code, identifying tokens. 
 * char ptr start - Points to the beginning of the current token.
 * char ptr current - Points to the current character being processed (probably in the middle of a token).
 * int line - Counts the line of source code currently being processed. 
 */
typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

Scanner scanner;


void init_scanner(const char* source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}


static bool char_digit(char ch) {
    return (ch >= '0' && ch <= '9');
}


static bool char_alpha(char ch) {
    return ( (ch >= 'a' && ch <= 'z') || 
             (ch >= 'A' && ch <= 'Z') ||
             (ch == '_') );
}


static bool at_file_end() {
    return (*scanner.current == '\0');
}


static char advance() {
    scanner.current++;
    return scanner.current[-1];
}


static char peek() {
    return (*scanner.current);
}


static char peek_ahead() {
    if (at_file_end()) {
        return '\0';
    }
    return scanner.current[1];
}


static bool match(char expected) {
    if (at_file_end()) {
        return false;
    }
    if (*scanner.current != expected) {
        return false;
    }

    scanner.current++;
    return true;
}

static Token create_token(TokenType of_type) {
    Token token;
    token.type = of_type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}


static Token error_token(const char* error_message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = error_message;
    token.length = (int)strlen(error_message);
    token.line = scanner.line;
    return token;
}


/*
 * Most whitespace has no semantic value in Cypsa. This value continues to advance as long as there is a space,
 * carriage return, or tab character. At a newline, the current line number is incremented.
 * Comments are also like whitespace in that they can be ignored. Double-slash // comments cause the rest of the
 * current line to be skipped.
 * If no whitespace or comment characters are found, then simply return.
 * TODO: Add multiline comments \/\* which proceed until the closing \*\/ is found (sry about the backslashes).
 */
static void skip_whitespace() {
    LOOP {
        char ch = peek();

        switch (ch) {
            // Whitespace we don't care about - spaces, line-feeds, carriage returns, and tabs.
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            // Handle newlines here too - just increase the line count by 1 and move to the next char
            case '\n':
                scanner.line++;
                advance();
                break;
            // C-style comments - if we see two forward-slashes then skip the rest of the current line
            // (or the end of the file, whichever comes first).
            case '/':
                if (peek_ahead() == '/') {
                    while ((peek() != '\n') && (!at_file_end())) {
                        advance();
                    }
                } else {
                    return;
                }
            default:
                return;
        }
    }
}


/*
 * Utility function which determines if some lexeme in the source code matches a reserved keyword. 
 * start:   the length of the first part of the string that matches; e.g., 'true' and 'this' both have a matching first
 *          letter, so start would be 1.
 * length:  the total length of the keyword tail without a terminator, so 'rue' would be 3, 'nd' 2, etc.
 * tail:    the remaining characters to be compared; in the first example, this would be 'rue' or 'his'.
 * of_type: the type of the ostensible keyword token to return if a match is found. Basically saying here, "if you find a
 *          match for this keyword, then this is the token I want you to give me back". If a match is not found then this 
 *          must be a simple identifier, so an IDENTIFIER token type is returned instead.
 */
static TokenType check_keyword(int start, int length, const char* tail, TokenType of_type) {
    int lexeme_length  = scanner.current - scanner.start;
    int keyword_length = start + length;
    int match_result   = memcmp(scanner.start + start, tail, length);

    if (lexeme_length == keyword_length && match_result == 0) {
            return of_type;
    }

    return TOKEN_IDENTIFIER;
}


/*
 * typeof_identifier: determines the type of an identifier, that is, whether it is a variable name or reserved keyword.
 * 
 * identifier: called when an identifier is... ahem, identified. The first character should always be alphabetical, but
 * any characters after that can be numerics - scanner.current is always pointing to at least the second character of a
 * token, so the check for digits can be made here (since, if this function was called, we know that it must have started
 * with an alphabetical letter).
 * This is accomplished via calls to check_keyword, which determines if some lexeme is a reserved keyword or not. If it is,
 * then it will return the token type of that keyword which is again returned by typeof_identifier(). Otherwise, the token
 * type will be a plain identifier.
 * The switch statements represent paths through a 'trie' of keyword strings which share starting letters. For example, an
 * 'f' may be the start of either 'false' or 'func'. check_keyword() takes the remaining keyword string and determines if
 * that exact keyword is present at this position in the source code.
 * The branching keywords first check if the length of the identifier is > 1. No keywords are a single letter long but
 * valid identifiers may be, so this bails out straight away and returns a plain identifier, saving some unnecessary work.
 */
static TokenType typeof_identifier() {
    switch (scanner.start[0]) {
        // Checks for non-branching keywords
        case 'a':
            return check_keyword(1, 2, "nd", TOKEN_AND);
        case 'c':
            return check_keyword(1, 4, "lass", TOKEN_CLASS);
        case 'e':
            return check_keyword(1, 3, "lse", TOKEN_ELSE);
        case 'i':
            return check_keyword(1, 1, "f", TOKEN_IF);
        case 'n':
            return check_keyword(1, 2, "il", TOKEN_NIL);
        case 'o':
            return check_keyword(1, 1, "r", TOKEN_OR);
        case 'p':
            return check_keyword(1, 4, "rint", TOKEN_PRINT);
        case 'r':
            return check_keyword(1, 5, "eturn", TOKEN_RETURN);
        case 's':
            return check_keyword(1, 4, "uper", TOKEN_SUPER);
        case 'v':
            return check_keyword(1, 2, "ar", TOKEN_VAR);
        case 'w':
            return check_keyword(1, 4, "hile", TOKEN_WHILE);
        
        // Checks for branching keywords
        case 'f':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'a':
                        return check_keyword(2, 3, "lse", TOKEN_FALSE);
                    case 'o':
                        return check_keyword(2, 1, "r", TOKEN_FOR);
                    case 'u':
                        return check_keyword(2, 2, "nc", TOKEN_FUNC);
                }
            }
            break;
        
        case 't':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'h':
                        return check_keyword(2, 2, "is", TOKEN_THIS);
                    case 'r':
                        return check_keyword(2, 2, "ue", TOKEN_TRUE);
                }
            }
            break;
    }

    return TOKEN_IDENTIFIER;
}


static Token identifier() {
    while (char_alpha(peek()) || char_digit(peek())) {
        advance();
    }

    return create_token(typeof_identifier());
}


/*
 * Create a token representing a numeric literal value. Advance while we are still in the number; to continue to parse
 * floating-points, skip over the '.' if found and continue to advance over the following digits. Once the end of the
 * number is reached, return a TOKEN_NUMBER to represent it.
 */
static Token number() {
    while (char_digit(peek())) {
        advance();
    }

    if (peek() == '.' && char_digit(peek_ahead())) {
        advance();
    }

    while (char_digit(peek())) {
        advance();
    }

    return create_token(TOKEN_NUMBER);
}


/*
 * Function to create tokens from string-literals. Similar to the whitespace and number functions - advances while not a
 * either the end of the string, or the end of the file. Newlines simply cause the scanners' line count to increase, which
 * allows for multiline strings in programs.
 */
static Token string() {
    while ((peek() != '"') && (!at_file_end())) {
        if (peek() == '\n') {
            scanner.line++;
        }
        advance();
    }

    if (at_file_end()) {
        return error_token("Error: Unterminated string-literal!");
    }

    advance();
    return create_token(TOKEN_STRING);
}


Token scan_token() {
    skip_whitespace();
    scanner.start = scanner.current;

    if (at_file_end()) {
        return create_token(TOKEN_EOF);
    }

    char ch = advance();

    if (char_alpha(ch)) {
        return identifier();
    }

    if (char_digit(ch)) {
        return number();
    }

    switch (ch) {
        // Tokens which are always 1 character in length
        case '(':
            return create_token(TOKEN_LEFTPAREN);
        case ')':
            return create_token(TOKEN_RIGHTPAREN);
        case '{':
            return create_token(TOKEN_LEFTCURLY);
        case '}':
            return create_token(TOKEN_RIGHTCURLY);
        case ';':
            return create_token(TOKEN_SEMICOLON);
        case ',':
            return create_token(TOKEN_COMMA);
        case '.':
            return create_token(TOKEN_DOT);
        case '-':
            return create_token(TOKEN_MINUS);
        case '+':
            return create_token(TOKEN_PLUS);
        case '/':
            return create_token(TOKEN_SLASH);
        case '*':
            return create_token(TOKEN_STAR);
        
        // Tokens which may either be 1 or 2 characters in length
        case '!':
            return create_token(
                (match('=') ? TOKEN_NOTEQUAL : TOKEN_EXCLAMATION)
            );
        case '=':
            return create_token(
                (match('=') ? TOKEN_EXACTEQUAL : TOKEN_EQUAL)
            );
        case '<':
            return create_token(
                (match('=') ? TOKEN_LESSEQUAL : TOKEN_LESS)
            );
        case '>':
            return create_token(
                (match('=') ? TOKEN_GREATEREQUAL : TOKEN_GREATER)
            );

        // String-literals: should always start with a double-quote "
        case '"':
            return string();
        
        default:
            return error_token("\nUnknown character found.");
    }

    return error_token("Something else went wrong!");
}

