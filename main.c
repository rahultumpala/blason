// a blazing fast json parser in C

#include "json.h"
#include "print.h"

int main(int argc, const char *argv[]) {
    ObjectJson *json = parseJSON("./test.json");
    print_json(json);
    return 0;
}