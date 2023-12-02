// a blazing fast json parser in C

#include "json.h"
#include "print.h"

int main(int argc, const char *argv[]) {
    ObjectJson *json = parseJSON("./test.json");
    print_json(json);

    Value *newValue = (Value *)malloc(sizeof(Value));
    newValue->next = NULL;
    newValue->type = VAL_NUMBER;
    newValue->as.number = 9999;

    blason_put(json, "testInsert", newValue);
    Value *val = blason_get(json, "testInsert");
    print_value(val);
    print_json_min(json);

    blason_put(json, "testInsert", blason_get(json, "testNum"));
    val = blason_get(json, "testInsert");
    print_value_min(val);
    print_json_min(json);

    return 0;
}