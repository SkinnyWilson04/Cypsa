#ifndef cypsa_memory_h
    #define cypsa_memory_h

    #include "common.h"

    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     * Macro: GROW_CAPACITY gives a starting capacity of 8 for empty nuggets. Otherwise, capacity grows by a factor of
     * two (8, 16, 32, 64, 128, etc.). This is called each time the current nugget capacity is full and needs to be expanded.
     */
    #define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)


    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     * Macro: GROW_ARRAY makes the call to reallocate() easier and less error-prone. 'type' is used to ensure that the size
     * of each element for any type is calculated correctly, and that the void* is also cast back to the correct type.
     * The actual work of reallocating the elements from the old_capacity to the new_ is handled by reallocate() itself.
     */
    #define GROW_ARRAY(type, pointer, old_capacity, new_capacity) (type*)reallocate(pointer, sizeof(type) * (old_capacity), \
    sizeof(type) * new_capacity)


    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     * Macro: FREE_ARRAY is another prettied-up call to the reallocate() function. Calling reallocate() with a new_capacity
     * of 0 causes the pointer to be freed.
     */
    #define FREE_ARRAY(type, pointer, old_capacity) (type*)reallocate(pointer, sizeof(type) * (old_capacity), 0)

    
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     * failed(): a simple wrapper around an assert to ensure that memory allocation has not failed / returned NULL.
     * reallocate(): handles allocation, freeing, and resizing of arrays. Based on the free() and realloc() functions. 
     */
    void check_failure(void* pointer, const char* message, size_t requested);
    void* reallocate(void* pointer, size_t old_size, size_t new_size);

#endif