// a blazing fast json parser in C

#include "json.h"
#include "print.h"

int main(int argc, const char *argv[]) {
    ObjectJson *json = parseJSON("./test.json");
    // print_json(json);
    Value *quiz = blason_get(json, "quiz");
    if(quiz != NULL){
        print_value(quiz);
    }else {
        printf("NULL recvd");
    }
    return 0;
}