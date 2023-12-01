#include "json.h"
#include <stdio.h>
#include <stdlib.h>

void __print_json(ObjectJson *json, int depth);
void print_value(Value value, int depth);

int width = 4;
bool __print_min = false;

void print_json_token(Token token) {
    printf("%.*s", token.length, token.value);
}

void print_obj(Object *obj, int depth) {
    switch (obj->type) {
    case OBJ_STRING: {
        ObjectString *str = (ObjectString *)obj;
        printf("%.*s", str->length, str->value);
        break;
    }
    case OBJ_JSON: {
        ObjectJson *json = (ObjectJson *)obj;
        __print_json(json, depth + 1);
        break;
    }
    case OBJ_ARRAY: {
        ObjectArray *array = (ObjectArray *)obj;
        printf("[");
        Value *t = array->start;
        while (t != NULL) {
            print_value(*t, depth);
            if (t->next == NULL) {
                break;
            }
            printf(",");
            t = t->next;
        }
        printf("]");
        break;
    }
    default:
        printf("PRINT NOT DEFINED FOR THIS OBJECT TYPE\n");
        break;
    }
}

void print_value(Value value, int depth) {
    switch (value.type) {
    case VAL_BOOL:
        printf("%s", value.as.boolean ? "true" : "false");
        break;
    case VAL_NIL:
        printf("null");
        break;
    case VAL_NUMBER: {
        long long n = (long long)value.as.number;
        if ((double)value.as.number - n == 0)
            printf("%lld", n);
        else
            printf("%.10f", value.as.number);
        break;
    }
    case VAL_OBJ:
        print_obj(value.as.obj, depth);
        break;
    }
}

void print_member(Member *member, int depth) {
    if (!__print_min)
        printf("%*c", depth * width, ' ');
    print_json_token(member->key);
    printf(" : ");
    print_value(member->value, depth);
    if (member->next) {
        if (!__print_min)
            printf(",\n");
        else
            printf(",");
        print_member(member->next, depth);
    }
}

void __print_json(ObjectJson *json, int depth) {
    if (!__print_min && json->members != NULL)
        printf("{\n");
    else
        printf("{");
    if (json->members != NULL)
        print_member(json->members, depth);

    if (!__print_min && json->members != NULL)
        printf("\n%*c}", depth * width, ' ');
    else
        printf("}");
}

void print_json(ObjectJson *json) {
    __print_json(json, 0);
}

void print_json_min(ObjectJson *json) {
    __print_min = true;
    __print_json(json, 0);
    __print_min = false;
}