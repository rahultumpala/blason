#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "json.h"

Lexer lexer = {
    .start = NULL,
    .current = NULL
};

Parser parser = {
    .previous = NULL,
    .current = NULL,
};

Value trueValue = {
    .type = VAL_BOOL,
    .next = NULL,
};

Value falseValue = {
    .type = VAL_BOOL,
    .next = NULL,
};

Value nullValue = {
    .type = VAL_NIL,
    .next = NULL,
};

Value tempVal = {
    .next = NULL,
};

Member *currentMember, *previousMember;
Value *valuePtr = &tempVal;

static char *readFile(const char *path){
    FILE *file;
    if(file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0l, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char *buffer = (char*)malloc(fileSize+1);
    if(buffer == NULL) {
        fprintf(stderr, "Could not allocate enough memory to read \"%s\".\n", path);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if(bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }

    buffer[bytesRead] = '\0'; // null byte

    fclose(file);
    return buffer;
}

static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool isDigit(char c) {
    return (c >= '0' && c <= '9');
}

static void skipWs() {
    while(*lexer.current != '\0' && 
    (*lexer.current == ' ' || 
     *lexer.current == '\r' || 
     *lexer.current == '\n' || 
     *lexer.current == '\t') ) lexer.current++;
}

static Token *createToken(TokenType type) {
    Token token = {
        .type = type,
        .length = (int) (lexer.current - lexer.start),
        .value = lexer.start
    };
    lexer.start = lexer.current;
    return &token;
}

static bool end(){
    return *lexer.current == '\0';
}

static Token *literal() {
    while(isAlpha(*lexer.current)) lexer.current++;
    int len = (int)(lexer.current - lexer.start);
    if(strncmp("true", lexer.start, len) == 0) return createToken(TRUE);
    if(strncmp("false", lexer.start, len) == 0) return createToken(FALSE);
    if(strncmp("null", lexer.start, len) == 0) return createToken(NIL);
    fprintf(stderr, "Unknown literal '%.*s'.\n", len, lexer.start);
    exit(74);
}

static Token *string() {
    while(!end()){
        if(*lexer.current == '"' && lexer.current[-1] != '\\') {
            lexer.current++;
            break;
        }
        lexer.current++; 
    }
    return createToken(STRING);
}

static Token *number() {
    while(!end()) {
        char c = *lexer.current;
        if(isDigit(c) || c == '.' || c == 'e' || c == 'E') lexer.current ++;
        else break;
    }
    // TODO: check if number is valid and conforming to the grammar
    return createToken(NUMBER);
}

static Token *scanToken() {
    skipWs();
    lexer.start = lexer.current;

    if(end()) return createToken(TOKEN_EOF);
    char c = *lexer.current++;

    if(isAlpha(c)) return literal();
    if(isDigit(c) || c == '-') return number();

    switch (c)
    {
        case '{' : return createToken(LBRACE);
        case '}' : return createToken(RBRACE);
        case '[' : return createToken(LSQUARE);
        case ']' : return createToken(RSQUARE);
        case ':' : return createToken(COLON);
        case ',' : return createToken(COMMA);
        case '"' : return string();
        default:
            fprintf(stderr, "Unknown character '%c'.\n", c);
            exit(74);
    }
}

static void lex(char *path) {
    lexer.start  = readFile(path);
    lexer.current = lexer.start;
}

static void advance(){
    parser.previous = parser.current;
    parser.current = scanToken();
}

static bool check(TokenType type) {
    return parser.current->type == type;
}

static bool match(TokenType type) {
    if(!check(type)) return false;
    advance();
    return true;
}

static void expect(TokenType type, char *message) {
    if(parser.current->type != type) {
        fprintf(stderr, message);
        exit(74);
    }
    advance();
}

static ObjectJson *object();
static void value();

static void elements() {
    Value newVal = {
        .next = valuePtr,
    };
    Value *cur = valuePtr;
    valuePtr = &newVal;
    value();
    cur->next = valuePtr;
    valuePtr = cur;
    while(match(COMMA)) elements();
}

static void array() {
    expect(LSQUARE, "Expect '[' at the beginning of an array.");
    elements();
    expect(RSQUARE, "Expect '[' at the beginning of an array.");
}

static void value() {
    switch (parser.current->type)
    {
        case TRUE:     
            valuePtr = &trueValue;
            break;
        case FALSE:
            valuePtr = &falseValue;
            break;
        case NIL:
            valuePtr = &nullValue;
            break;
        case NUMBER: {
            Value numberValue = {
                .type = VAL_NUMBER,
                .as = strtod(parser.current->value, NULL),
            };
            valuePtr = &numberValue;
            break;
        }
        case STRING: {
            ObjectString string = {
                .length = parser.current->length,
                .type = OBJ_STRING,
                .value = parser.current->value
            };
            Value value = {
                .type = VAL_OBJ,
                .as = &string,
            };
            valuePtr = &value;
        }
        case LBRACE: {
            Member *ptr = currentMember;
            Value value = {
                .type = VAL_OBJ,
                .as = object()
            };
            currentMember = ptr;
            valuePtr = &value;
        }
        case LSQUARE: {

        }
    }
}

static void pair() {
    expect(STRING, "Expect string as key in a pair.");
    currentMember->key = *parser.previous;
    expect(COLON, "Expect ':' between key and value in a pair.");
    value();
    currentMember->value = *valuePtr;
}

static void members() {
    previousMember = currentMember;
    Member newVal = {
        .next = NULL
    };
    currentMember = &newVal;
    pair();
    previousMember->next = currentMember;
    currentMember = previousMember;
    while(match(COMMA)) members();
}

static ObjectJson *object() {

    Member member = {
        .key = NULL,
        .value = NULL,
        .next = NULL
    };
    currentMember = &member;

    ObjectJson json = {
        .type = OBJ_JSON,
        .members = &member,
    };

    expect(LBRACE, "Expect '{' at the beginning.");
    if(!match(RBRACE)) members();
    expect(RBRACE, "Expect '}' after members.");
    
    return &json;
}

ObjectJson parseJSON(char *path) {
    lex(path);
    return *object();
}