#ifndef blason
#define blason

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

struct Member;
struct Value;

typedef struct {
    ObjectType type;
    int length;
    Value *start;
} ObjectArray;

typedef struct {
    ObjectType type;
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
    Value value;
    struct Member *next;
};

typedef struct Member Member;
typedef struct Value Value;

ObjectJson parseJSON(char *path);

#endif