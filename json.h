#ifndef blason
#define blason

#include <stdbool.h>

typedef enum {
    LBRACE,
    RBRACE,
    LSQUARE,
    RSQUARE,
    NUMBER,
    STRING,
    COLON,
    COMMA,
    TRUE,
    FALSE,
    NIL,
    TOKEN_EOF,
} TokenType;

typedef struct {
    TokenType type;
    int length;
    char *value;
} Token;

typedef struct {
    char *start;
    char *current;
} Lexer;

typedef struct {
    Token *previous;
    Token *current;
} Parser;

typedef enum {
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_OBJ,
} ValueType;

typedef enum {
    OBJ_STRING,
    OBJ_ARRAY,
    OBJ_JSON
} ObjectType;

typedef struct {
    ObjectType type;
    int length;
    char *value;
} ObjectString;

typedef struct Member Member;
typedef struct Value Value;

typedef struct {
    ObjectType type;
    int length;
    Value *start;
} ObjectArray;

typedef struct {
    ObjectType type;
    struct bst *htable;
    Member *members;
} ObjectJson;

typedef struct {
    ObjectType type;
} Object;

// tagged union
struct Value {
    ValueType type;
    union {
        bool boolean;
        double number;
        Object *obj;
    } as;
    struct Value *next;
};

struct Member {
    Token key;
    Value *value;
    struct Member *next;
};

typedef struct bst bst;

struct bst {
    long long hash;
    Value *value;
    bst *left;
    bst *right;
};

ObjectJson *parseJSON(char *path);

unsigned long long create_hash(char *key, int length);

void create_bst(ObjectJson *json);

void insert_bst(bst *root, unsigned long long hash, Value *value);

void delete_bst(unsigned long long hash);

bst *fetch_bst(bst *root, unsigned long long hash);

Value *blason_get(ObjectJson *json, char *key);

void blason_put(ObjectJson *json, char *key, Value *val);

#define DEBUG_ERRORS

#endif