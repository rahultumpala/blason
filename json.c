#include "json.h"
#include "print.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Lexer lexer = {
    .start = NULL,
    .current = NULL};

Parser parser = {
    .previous = NULL,
    .current = NULL,
};

Value trueValue = {
    .type = VAL_BOOL,
    .as = true,
    .next = NULL,
};

Value falseValue = {
    .type = VAL_BOOL,
    .as = false,
    .next = NULL,
};

Value nullValue = {
    .type = VAL_NIL,
    .next = NULL,
};

static char *readFile(const char *path) {
    FILE *file;
    file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0l, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Could not allocate enough memory to read \"%s\".\n", path);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
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
    while (*lexer.current != '\0' &&
           (*lexer.current == ' ' ||
            *lexer.current == '\r' ||
            *lexer.current == '\n' ||
            *lexer.current == '\t'))
        lexer.current++;
}

static Token *createToken(TokenType type) {
    Token *token = (Token *)malloc(sizeof(Token));
    token->type = type,
    token->length = (int)(lexer.current - lexer.start),
    token->value = lexer.start;
    lexer.start = lexer.current;
    // printf("creating Token: type= %d, length= %d value= %.*s\n", token->type, token->length, token->length, token->value);
    return token;
}

static bool end() {
    return *lexer.current == '\0';
}

static Token *literal() {
    while (isAlpha(*lexer.current))
        lexer.current++;
    int len = (int)(lexer.current - lexer.start);
    if (strncmp("true", lexer.start, len) == 0)
        return createToken(TRUE);
    if (strncmp("false", lexer.start, len) == 0)
        return createToken(FALSE);
    if (strncmp("null", lexer.start, len) == 0)
        return createToken(NIL);
    fprintf(stderr, "Unknown literal '%.*s'.\n", len, lexer.start);
    exit(74);
}

static Token *string() {
    while (!end()) {
        if (*lexer.current == '"' && lexer.current[-1] != '\\') {
            lexer.current++;
            break;
        }
        lexer.current++;
    }
    return createToken(STRING);
}

static Token *number() {
    while (!end()) {
        char c = *lexer.current;
        if (isDigit(c) || c == '.' || c == 'e' || c == 'E')
            lexer.current++;
        else
            break;
    }
    // TODO: check if number is valid and conforming to the grammar
    return createToken(NUMBER);
}

static Token *scanToken() {
    skipWs();
    lexer.start = lexer.current;

    if (end())
        return createToken(TOKEN_EOF);
    char c = *lexer.current++;

    if (isAlpha(c))
        return literal();
    if (isDigit(c) || c == '-')
        return number();

    switch (c) {
    case '{':
        return createToken(LBRACE);
    case '}':
        return createToken(RBRACE);
    case '[':
        return createToken(LSQUARE);
    case ']':
        return createToken(RSQUARE);
    case ':':
        return createToken(COLON);
    case ',':
        return createToken(COMMA);
    case '"':
        return string();
    default:
        fprintf(stderr, "Unknown character '%c'.\n", c);
        exit(74);
    }
}

static void lex(char *path) {
    lexer.start = readFile(path);
    lexer.current = lexer.start;
}

static void advance() {
    parser.previous = parser.current;
    parser.current = scanToken();
}

static bool check(TokenType type) {
    return parser.current->type == type;
}

static bool match(TokenType type) {
    if (!check(type))
        return false;
    advance();
    return true;
}

static void expect(TokenType type, char *message) {
    if (parser.current->type != type) {
#ifdef DEBUG_ERRORS
        printf("previous type: %d, previous length: %d, previous value : %.*s\n", parser.previous->type, parser.previous->length, parser.previous->length, parser.previous->value);
        printf("Current length: %d, Current value : %.*s\n", parser.current->length, parser.current->length, parser.current->value);
        printf("current type  : %d, expected type : %d\n", parser.current->type, type);
        advance();
        printf("Next length: %d, Next value : %.*s\n", parser.current->length, parser.current->length, parser.current->value);
#endif
        fprintf(stderr, "%s\n", message);
        exit(74);
    }
    advance();
}

static ObjectJson *object();
static Value *value();

static Value *elements() {
    Value *start = value();
    Value *end = start;
    while (match(COMMA)) {
        end->next = value();
        end = end->next;
    }
    if (end != NULL)
        end->next = NULL;
    return start;
}

static ObjectArray *array() {
    expect(LSQUARE, "Expect '[' at the beginning of an array.");
    ObjectArray *arrayObj = (ObjectArray *)malloc(sizeof(ObjectArray));
    arrayObj->type = OBJ_ARRAY;
    if (check(RSQUARE))
        arrayObj->start = NULL;
    else
        arrayObj->start = elements();
    expect(RSQUARE, "Expect '[' at the beginning of an array.");
    return arrayObj;
}

static Value *value() {
    switch (parser.current->type) {
    case TRUE:
        advance();
        return &trueValue;
        break;
    case FALSE:
        advance();
        return &falseValue;
        break;
    case NIL:
        advance();
        return &nullValue;
        break;
    case NUMBER: {
        Value *numberValue = (Value *)malloc(sizeof(Value));
        numberValue->type = VAL_NUMBER;
        numberValue->as.number = (double)strtod(parser.current->value, NULL);
        advance();
        return numberValue;
        break;
    }
    case STRING: {
        ObjectString *string = (ObjectString *)malloc(sizeof(ObjectString));
        string->type = OBJ_STRING;
        string->length = parser.current->length;
        string->value = parser.current->value;

        Value *value = (Value *)malloc(sizeof(Value));
        value->type = VAL_OBJ;
        value->as.obj = (Object *)string;

        advance();
        return value;
        break;
    }
    case LBRACE: {
        Value *value = (Value *)malloc(sizeof(Value));
        value->type = VAL_OBJ;
        value->as.obj = (Object *)object();
        return value;
        break;
    }
    case LSQUARE: {
        ObjectArray *arrayObj = array();

        Value *value = (Value *)malloc(sizeof(Value));
        value->type = VAL_OBJ;
        value->as.obj = (Object *)arrayObj;

        return value;
        break;
    }
    default:
        fprintf(stderr, "%s", "Unsupported value type.");
        exit(74);
    }
}

static void pair(Member *member) {
    expect(STRING, "Expect string as key in a pair.");
    member->key = *parser.previous;
    expect(COLON, "Expect ':' between key and value in a pair.");
    member->value = *value();
}

static Member *members() {
    Member *member = (Member *)malloc(sizeof(Member));
    member->next = NULL;
    pair(member);
    while (match(COMMA)) {
        member->next = members();
    }
    return member;
}

static ObjectJson *object() {
    if (!check(LBRACE))
        return NULL;

    ObjectJson *json = (ObjectJson *)malloc(sizeof(ObjectJson));
    json->type = OBJ_JSON;

    expect(LBRACE, "Expect '{' at the beginning.");
    if (check(RBRACE))
        json->members = NULL;
    else
        json->members = members();
    expect(RBRACE, "Expect '}' after members.");

    return json;
}

ObjectJson *parseJSON(char *path) {
    lex(path);
    // to set the first token in parser.current
    advance();
    return object();
}

unsigned long long create_hash(char *key, int length) {
    unsigned long long p = 31;
    unsigned long long hash = 0;
    for (int i = 0; i < length; i++) {
        // if long then let overflow, it is equivalent to using a 2^64 module
        hash += p * (key[i] - 'a' + 1);
        p *= p;
    }
    return hash;
}

static bst *__create_node() {
    bst *ROOT = (bst *)malloc(sizeof(bst));
    ROOT->value = NULL;
    ROOT->left = NULL;
    ROOT->right = NULL;
    ROOT->hash = NULL;
    return ROOT;
}

void insert_bst(bst *root, unsigned long long hash, Value *val) {
    if (root->val == NULL) {
        root->hash = hash;
        root->value = val;
        return;
    }
    if (root->hash > hash) {
        if (root->left == NULL)
            root->left = __create_node();
        insert_bst(root->left, hash, val);
    } else if (root->hash < hash) {
        if (root->right == NULL)
            root->right = __create_node();
        insert_bst(root->right, hash, val);
    }
}

void create_bst(ObjectJson *json) {
    if (json->htable != NULL)
        return;
    bst *ROOT = __create_node();
    Member *cur = json->members;
    while (cur != NULL) {
        unsigned long long hash = create_hash(cur->key.value, cur->key.length);
        insert_bst(ROOT, hash, cur->value);
        if (cur->value.type == VAL_OBJ) {
            Object *obj = (Object *)cur->value.as.obj;
            if (obj->type == OBJ_JSON)
                create_bst((ObjectJson *)obj);
        }
        cur = cur->next;
    }
    // todo: rebalance before assigning
    json->htable = ROOT;
}

bst *fetch_bst(bst *root, unsigned long long hash) {
    if (root == NULL)
        return NULL;
    if (root->hash == hash)
        return root;
    if (root->hash > hash)
        return fetch_bst(root->left, hash);
    return fetch_bst(root->right, hash);
}

Value *blason_get(ObjectJson *json, char *key) {
    if (json == NULL)
        return NULL;
    if (json->htable == NULL || key == NULL)
        return NULL;
    int len = strlen(key);
    unsigned long long hash = create_hash(key, len);
    bst *node = fetch_bst(json->htable, hash);
    if (!node)
        return NULL;
    return node->value;
}
