#include <string.h>
#include <stdio.h>
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
    file = fopen(path, "rb");
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
    Token *token = (Token *) malloc(sizeof(Token));
    token->type = type,
    token->length = (int) (lexer.current - lexer.start),
    token->value = lexer.start;
    lexer.start = lexer.current;
    return token;
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
        #ifdef DEBUG_ERRORS
            printf("previous type: %d, previous length: %d, previous value : %.*s\n",parser.previous->type, parser.previous->length, parser.previous->length, parser.previous->value);
            printf("Current length: %d, Current value : %.*s\n",parser.current->length, parser.current->length, parser.current->value);
            printf("current type  : %d, expected type : %d\n",parser.current->type, type);
            advance();
            printf("Next length: %d, Next value : %.*s\n",parser.current->length, parser.current->length, parser.current->value);
        #endif
        fprintf(stderr, "%s\n", message);
        exit(74);
    }
    advance();
}

static ObjectJson *object();
static void value();

static void elements() {
    Value *newVal = (Value *)malloc(sizeof(Value));
    newVal->next = valuePtr;
    Value *cur = valuePtr;
    valuePtr = newVal;
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
            advance();
            break;
        case FALSE:
            valuePtr = &falseValue;
            advance();
            break;
        case NIL:
            valuePtr = &nullValue;
            advance();
            break;
        case NUMBER: {
            Value *numberValue = (Value *)malloc(sizeof(Value));
            numberValue->type = VAL_NUMBER;
            numberValue->as.number = (double) strtod(parser.current->value, NULL);
            valuePtr = numberValue;
            advance();
            break;
        }
        case STRING: {
            ObjectString *string = (ObjectString *)malloc(sizeof(ObjectString));
            string->type = OBJ_STRING;
            string->length = parser.current->length;
            string->value = parser.current->value;

            Value *value = (Value *)malloc(sizeof(Value));
            value->type = VAL_OBJ;
            value->as.obj = (Object *) string;

            valuePtr = value;
            advance();
            break;
        }
        case LBRACE: {
            Member *ptr = currentMember;
            Value *value = (Value *)malloc(sizeof(Value));
            value->type = VAL_OBJ;
            value->as.obj = (Object *) object();
            
            currentMember = ptr;
            valuePtr = value;
            break;
        }
        case LSQUARE: {
            ObjectArray *arrayObj = (ObjectArray *)malloc(sizeof(ObjectArray));
            arrayObj->type = OBJ_ARRAY;
            arrayObj->start = valuePtr;

            array();
            
            Value *value = (Value *)malloc(sizeof(Value));
            value->type = VAL_OBJ;
            value->as.obj = (Object *) arrayObj;

            valuePtr = value;
            break;
        }
        default:
            fprintf(stderr, "%s", "Unsupported value type.");
            exit(74);
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
    Member *newVal = (Member *)malloc(sizeof(Member));
    newVal->next = NULL;
    currentMember = newVal;
    pair();
    previousMember->next = currentMember;
    currentMember = previousMember;
    while(match(COMMA)){
        members();
    } 
}

static ObjectJson *object() {

    Member *member = (Member *)malloc(sizeof(Member));
    member->next = NULL;

    previousMember = currentMember;
    currentMember = member;

    ObjectJson *json = (ObjectJson *) malloc(sizeof(ObjectJson));
    json->type = OBJ_JSON;
    json->members = member;

    expect(LBRACE, "Expect '{' at the beginning.");
    if(!check(RBRACE)) members();
    expect(RBRACE, "Expect '}' after members.");

    currentMember = previousMember;
    return json;
}

ObjectJson parseJSON(char *path) {
    lex(path);
    // to set the first token in parser.current
    advance();
    return *object();
}