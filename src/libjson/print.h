#ifndef blason_print
#define blason_print

#include <stdio.h>
#include <stdlib.h>

#include "json.h"

void print_json_token(Token token);

void __print_obj(Object *obj, int depth);

void print_obj(Object *obj);

void __print_value(Value *value, int depth);

void print_value(Value *value);

void print_value_min(Value *value);

void print_member(Member *member, int depth);

void __print_json(ObjectJson *json, int width);

void print_json(ObjectJson *json);

void print_json_min(ObjectJson *json);

#endif // blason_print