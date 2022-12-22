#ifndef cypsa_values_h
    #define cypsa_values_h

    #include "common.h"

    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     * All values in Cypsa are real, double-precision floating-point numbers. 
     * The ValuePool struct stores Cypsa Values in a dynamic array whose implementation almost exactly mirrors that of the
     * nugget code dynamic array. It makes use of the same GROW_CAPACITY, GROW_ARRAY, and FREE_ARRAY macros that nugget does.
     * capacity stores the current total size of the values array, while occupied, obviously, stores the number which are in use.
     */ 
    typedef double Value;

    typedef struct {
        int capacity;
        int occupied;
        Value* values;
    } ValuePool;

    void init_valuepool(ValuePool* pool);
    void write_valuepool(ValuePool* pool, Value value);
    void free_valuepool(ValuePool* pool);
    void print_value(Value value);

#endif