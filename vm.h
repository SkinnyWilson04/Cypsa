#ifndef cypsa_vm_h
    #define cypsa_vm_h

    #include "nugget.h"
    #include "values.h"

    #define STACK_MAXSIZE 8
    

    /* This is what the old stack looked like - advantages being that it had a known size at compile-time, and the
     * array was right there inline in the struct. Disadvantages being that the STACK_MAX had to be a constant size
     * and the array could not grow. The new implementation contains a pointer which needs to be freed, and is likely
     * a little slower, but has a bounds check, is small for simple programs, and can grow as complexity increases. 
     * typedef struct {
     *     Nugget*  nugget;
     *     uint8_t* iptr;
     *     Value stack[STACK_MAXSIZE];
     *     Value* stack_top;
     * } VM; */
    
    typedef struct {
        Nugget* nugget;
        uint8_t* iptr;
        Value* stack;
        Value* stack_ptr;
        Value* stack_top;
        int stack_capacity;
    } VM;

    typedef enum {
        INTERPRETER_OK,
        INTERPRETER_COMPILE_ERROR,
        INTERPRETER_RUNTIME_ERROR
    } InterpretationResult;

    void init_VM(void);
    void free_VM(void);
    InterpretationResult interpret(const char* source);
    void push(Value value);
    Value pop();

#endif