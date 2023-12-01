#ifndef blason_print
#define blason_print


#include <stdlib.h>
#include <stdio.h>

#include "json.h"


void print_json_token(Token token);

void print_obj(Object *obj, int depth);

void print_value(Value value, int depth);

void print_member(Member *member, int depth);

void __print_json(ObjectJson *json, int width);

void print_json(ObjectJson *json);

void print_json_min(ObjectJson *json);

#endif // blason_print