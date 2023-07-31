// a blazing fast json parser in C

#include "json.h"

int main(int argc, const char *argv[]) {
    ObjectJson json = parseJSON("./test.json");
    return 0;
}