#include <stdlib.h>
#include <stdio.h>
#include "json.h"

void print_json_token(Token token){
    // printf("Token: type= %d, length= %d value= %.*s\n", token.type, token.length, token.length, token.value);
    printf("%.*s", token.length, token.value);
}

void print_obj(Object *obj){
    switch (obj->type)
    {
    case OBJ_STRING:
        printf("%.*s");
        break;
    default:
        break;
    }
}

void print_value(Value value){
    switch (value.type)
    {
    case VAL_BOOL:
        printf("%s\n", value.as.boolean ? "true" : "false");
        break;
    case VAL_NIL:
        printf("nil\n");
        break;
    case VAL_NUMBER:
        printf("%.10f\n", value.as.number);
        break;
    case VAL_OBJ:
        print_obj(value.as.obj);
        break;
    }
}

void print_member(Member *member, int depth){
    printf("%*c", depth, ' ');
    print_json_token(member->key);
    printf(" : ");
    printf("%*c", depth, ' ');
}

void print_json(ObjectJson json){
    printf("{\n");
    if(json.members != NULL)
        print_member(json.members, 1);
    printf("\n}\n");
}