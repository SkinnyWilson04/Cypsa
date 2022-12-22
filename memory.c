#include <stdlib.h>
#include "memory.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Ensure memory allocation succeeded. The ASSERT_FORMAT macro simplifies formatted error
 * messages in any assertions that fail.
 */
#define ASSERT_FORMAT(Ast, Msg, Sz)                                          \
if(!(Ast)) {                                                                 \
    fprintf(stdout, "[FAILURE]: %s (requested %I64u bytes)\n", Msg, Sz);     \
    assert(Ast);                                                             \
}

void check_failure(void* pointer, const char* message, size_t requested) {
    ASSERT_FORMAT(pointer != NULL, message, requested);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * The reallocate function will handle all memory allocation and deallocation for Cypsa. This will be
 * important when we come to implement the garbage collector, and need to keep track of how much memory
 * is currently allocated.
 * Firstly, check the deallocation case - if the new size is zero, then we need to free() the array manually.
 * If the old size is 0 and the new size non-zero, then we want to allocate a new block.
 * A non-zero old size with a *smaller* new size indicates we need to shrink the code array.
 * Otherwise (likely the most common case) new_size will be *larger*, which requires growing the code array.
 */
void* reallocate(void* pointer, size_t old_size, size_t new_size) {
    if (new_size == 0) {
        free(pointer);
        return NULL;
    }

    void* resized = realloc(pointer, new_size);
    check_failure(resized, "Unable to reallocate code chunk.", new_size);
    return resized;
}