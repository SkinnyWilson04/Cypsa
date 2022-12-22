#ifndef cypsa_scanner_h
    #define cypsa_scanner_h

    /*
     * Tokens, special characters, and keywords that the scanner will recognize as tokens.
     * To avoid each token having a unique string representing it, all tokens in the program will point
     * into the original source string where that token begins (token.start) along with the number of characters
     * that they occupy. This prevents any unusual conflicts of ownership and risk of double-frees - here, we only
     * need to free the original source string once at the end of program execution.
     */
    typedef enum {
        TOKEN_LEFTPAREN, TOKEN_RIGHTPAREN, TOKEN_LEFTCURLY, TOKEN_RIGHTCURLY,
        TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS, TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,
        TOKEN_EXCLAMATION, TOKEN_NOTEQUAL, TOKEN_EQUAL, TOKEN_EXACTEQUAL, TOKEN_GREATER, TOKEN_GREATEREQUAL,
        TOKEN_LESS, TOKEN_LESSEQUAL,
        TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,
        TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE, TOKEN_FOR, TOKEN_FUNC, TOKEN_IF, TOKEN_NIL, TOKEN_OR,
        TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS, TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,
        TOKEN_ERROR, TOKEN_EOF 
    } TokenType;


    typedef struct {
        TokenType type;
        const char* start;
        int length;
        int line;
    } Token;


    void init_scanner(const char* source);
    Token scan_token();

#endif