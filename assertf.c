#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define ASSERT_FORMAT(Ast, Msg)                 \
if(!(Ast)) {                                    \
    fprintf(stdout, "[FAILURE] %s", Msg);       \
}


void assertf(void* pointer, const char* message) {
    ASSERT_FORMAT(pointer != NULL, message);
}

int main(void) {
    char* message = {"Well well well."};

    void* ptr = NULL;

    assertf(ptr, message);

    return 0;
}