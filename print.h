#ifndef blason_print
#define blason_print


#include <stdlib.h>
#include <stdio.h>

#include "json.h"


void print_json_token(Token token);

void print_member(Member *member, int depth);

void print_json(ObjectJson json);

#endif // blason_print