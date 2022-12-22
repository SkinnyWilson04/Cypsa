/* 
 * ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ *
 * Our intermediate representation of bytecode will contain a dense, linear sequence of instructions which will be run on
 * an emulator that we write. This will make it faster than directly-interpreted code (but slower than a native machine-
 * code application). Blocks of bytecode are Nuggets. They are mostly wrappers around dynamically-growing arrays of bytes
 * which contain the bytecode opcodes themselves. We'll stick with the way C++ handles dynamic Vectors for now - start with
 * some sensible small size (8 instructions) and then reallocate to an array 2x the size each time we hit maximum capacity.
 * 
 * enum OpCode:
 *      Defines the enums which control instructions to be executed
 * 
 * struct Nugget:
 *      A dynamically-growing array of bytes which contains bytecode instructions.
 *      int occupied:   The number of bytecode positions in the array currently occupied.
 *      int capacity:   The total current capacity of the bytecode array.
 * 
 * The Nugget.occupied and .capacity values are signed instead of unsigned to avoid any unusual sign-removal rules a compiler
 * might implement when handling arithmetic between signed and unsigned types, as explained in Expert C Programming: Deep C
 * Secrets (2nd Ed.) - it's around page 40 or so, I can't exactly remember to be honest.
 * ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ *
 */
#ifndef cypsa_nugget_h
    #define cypsa_nugget_h

    #include "common.h"
    #include "values.h"

    /*
     * ~ ~ ~ ~ ~ ~ ~ ~ ~ *
     */

    typedef enum {
        OPCODE_CONSTANT,
        OPCODE_CONSTANT_LONG,
        OPCODE_NEGATE,
        OPCODE_ADD,
        OPCODE_SUBTRACT,
        OPCODE_MULTIPLY,
        OPCODE_DIVIDE,
        OPCODE_RETURN
    } OpCode;

    typedef struct {
        int occupied;
        int capacity;
        uint8_t* code;
        size_t* lines;
        ValuePool constants;
    } Nugget;

    typedef uint32_t LongConstant;

    /*
     * ~ ~ ~ ~ ~ ~ ~ ~ ~ *
     */

    void init_nugget(Nugget* nugget);
    void free_nugget(Nugget* nugget);
    void write_nugget(Nugget* nugget, uint8_t byte, size_t line);
    int add_constant(Nugget* nugget, Value value);
    void write_constant(Nugget* nugget, Value value, int line);

#endif