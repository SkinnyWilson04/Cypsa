#include <stdio.h>
#include "compiler.h"
#include "common.h"
#include "debug.h"
#include "memory.h"
#include "values.h"
#include "vm.h"


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Create a fresh virtual machine struct (eww, global state). 
 * Initialize the VM - set the stack's capacity to zero and NULL out all of the pointers into the stack array.
 * rewind_stack() - Simple utility function which moves the stack pointer back to the beginning of the stack array. No need
 *                  to do anything with the existing values, since they will be overwritten with use.
 * stack_offset() - The difference between the bottom of the stack and the stack slot to be written to next. Useful when
 *                  shuffling pointers around during reallocation. 
 */
VM vm;

static void rewind_stack() {
    vm.stack_ptr = vm.stack;
}


void init_VM(void) {
    vm.stack_capacity = 0;
    vm.stack = NULL;
    vm.stack_ptr = vm.stack;
    vm.stack_top = vm.stack;
}


static inline int stack_offset(void) {
    return (int)(vm.stack_ptr - vm.stack);
}


void free_VM(void) {

}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Resizable stack operations. If we're currently pointing to the very top of the stack, then it is full and requires resizing.
 * First, get the location of the current stack index using stack_offset; this will tell us where to point back into the
 * resized array, incase the whole thing gets reallocated and moved. Grow the capacity and stack itself, move the stack_top
 * pointer to the very top of the new stack, and then point the stack_ptr back into it at the current index.
 * Stick in the incoming value and increment.
 */
void push(Value value) {
    if (vm.stack_ptr == vm.stack_top) {
        int stack_current = stack_offset();
        int prev_capacity = vm.stack_capacity;
        vm.stack_capacity = GROW_CAPACITY(vm.stack_capacity);
        vm.stack          = GROW_ARRAY(Value, vm.stack, prev_capacity, vm.stack_capacity);
        vm.stack_top      = &(vm.stack[vm.stack_capacity]);
        vm.stack_ptr      = &(vm.stack[stack_current]);
    }

    *vm.stack_ptr = value;
    vm.stack_ptr++;
}

Value pop() {
    vm.stack_ptr--;
    return (*vm.stack_ptr);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Where the bulk of the processing time will be spent. The FETCH_BYTE macro dereferences the current byte from the instruction
 * pointer and then increments it. The VM loop switches on the first byte of the instruction, which is always its OPCODE.
 * The opcode operands are then dispatched to the corresponding C implementation.
 * Since the instruction pointer increments immediately after fetching a byte, the pointer will always be pointing to the *next*
 * byte of code to be used, not the current byte.
 * When interpretation ends, run() will return a status enum back to the caller to indicate whether execution was successful,
 * or whether there was a compile-time or runtime error.
 * ~ ~ NOTE:
 *           There are apparently many ways of dispatching bytecode that are much more sophisticated and efficient than
 *           a switch statement, but we're obviously not using any of them. It would be interesting to research a couple
 *           for potential future runtime improvements.
 * 
 * The DEBUG_TRACE_EXECUTION flag is defined in common.h. If it's there, instructions will be disassembled and displayed as the
 * interpreter runs. If you don't need this debug information, simply comment out the #define for this in common.h
 * The current state of the stack will be displayed from the bottom up which, while verbose, is useful for sanity-checking execution.
 * disassemble_instruction() requires an offset, so the difference between the instruction currently pointed to by the instruction
 * pointer and the start of the nugget code array is calculated and cast to an int.
 * The BINARY_OPERATION macro is a piece of preprocessor black magic. The do-while block ensures that all of the statements are
 * in the same scope, and that the preprocessor replaces them in the code correctly. Making the condition 'false' ensures it
 * only run once, but that everything inside the block is in the same scope.
 * It saves writing a separate function with the logic to handle building and evaluating each expression, but I don't like it.
 */
static InterpretationResult run() {
    // Some helpful macros, for this scope only - definitions are removed at the end of run()
    #define FETCH_BYTE() (*vm.iptr++)
    #define FETCH_CONSTANT() (vm.nugget->constants.values[FETCH_BYTE()])
    #define BINARY_OPERATION(operation) \
        do {                            \
            double r = pop();           \
            double l = pop();           \
            push(l operation r);        \
        } while (false)


    // Main virtual machine fetch-decode-dispatch loop
    LOOP {
        #ifdef DEBUG_TRACE_EXECUTION
            printf("        ");
            for (Value* index = vm.stack; index < vm.stack_ptr; index++) {
                printf("[");
                print_value((*index));
                printf("]");
            }
            printf("\n");
            disassemble_instruction(vm.nugget, (int)(vm.iptr - vm.nugget->code));
        #endif


        // Opcode matching
        uint8_t instruction;
        
        switch (instruction = FETCH_BYTE()) {
            case OPCODE_RETURN: {
                print_value(pop());
                printf("\n");
                return INTERPRETER_OK;
            }

            case OPCODE_NEGATE: {
                *(vm.stack_ptr - 1) = 0 - (*(vm.stack_ptr - 1));
                break;
            }

            case OPCODE_ADD: {
                BINARY_OPERATION(+);
                break;
            }

            case OPCODE_SUBTRACT: {
                BINARY_OPERATION(-);
                break;
            }

            case OPCODE_DIVIDE: {
                BINARY_OPERATION(/);
                break;
            }

            case OPCODE_MULTIPLY: {
                BINARY_OPERATION(*);
                break;
            }

            case OPCODE_CONSTANT: {
                Value constant = FETCH_CONSTANT();
                push(constant);
                break;
            }
        }
    }

    #undef FETCH_BYTE
    #undef FETCH_CONSTANT
    #undef BINARY_OPERATION
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Begin interpreting and running the given code nugget. Returns the status 
 */
InterpretationResult interpret(const char* source) {
    Nugget nugget;
    init_nugget(&nugget);

    if (!compile(&nugget, source)) {
        free_nugget(&nugget);
        return INTERPRETER_COMPILE_ERROR;
    }

    vm.nugget = &nugget;
    vm.iptr   = vm.nugget->code;

    InterpretationResult interp_result = run();

    free_nugget(&nugget);

    return interp_result;
}

