#include <stdlib.h>
#include "nugget.h"
#include "memory.h"

#define HIGH_BYTE(x) (((x) & 0xFF000000) >> 24)
#define HMID_BYTE(x) (((x) & 0x00FF0000) >> 16)
#define LMID_BYTE(x) (((x) & 0x0000FF00) >> 8)
#define LOW_BYTE(x)  (((x) & 0x000000FF))

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Set the given code nugget back to a fresh empty state
 */
void init_nugget(Nugget* nugget) {
    nugget->occupied = 0;
    nugget->capacity = 0;
    nugget->code     = NULL;
    nugget->lines    = NULL;
    init_valuepool(&nugget->constants);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Take the nugget to Sweden.
 * Macro: FREE_ARRAY is *another* wrapper around reallocate() which calls it with a new_size of 0,
 *        which causes reallocate() to free the memory from *array.
 * init_nugget() is then called which zeroes the fields and pointer, so nugget is in a known-fresh state
 * constants, the pool of constant values, is a struct which also contains its own array. When we free the
 * code array, we also need to ensure the constant values are freed by calling free_valuepool
 */
void free_nugget(Nugget* nugget) {
    FREE_ARRAY(uint8_t, nugget->code, nugget->capacity);
    FREE_ARRAY(size_t, nugget->lines, nugget->capacity);
    free_valuepool(&nugget->constants);
    init_nugget(nugget);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Add a new bytecode byte to the end of the codeblock in this nugget. First, we need to check if
 * the number of occupied bytecode slots have hit max capacity. If we have, we'll grow the capacity
 * by 2x using the GROW_CAPACITY macro, and then reallocate the code to a 2x larger array using the
 * GROW_ARRAY macro. Then, stick the new byte onto the end of the list and increment the occupied
 * count. If we don't need to do any of the growing and reallocation, we'll just push and increment
 */
void write_nugget(Nugget* nugget, uint8_t byte, size_t line) {
    if (nugget->capacity < (nugget->occupied + 1)) {
        int prev_capacity = nugget->capacity;
        nugget->capacity  = GROW_CAPACITY(nugget->capacity);
        nugget->code      = GROW_ARRAY(uint8_t, nugget->code, prev_capacity, nugget->capacity);
        nugget->lines     = GROW_ARRAY(size_t, nugget->lines, prev_capacity, nugget->capacity);
    }

    nugget->code[nugget->occupied]  = byte;
    nugget->lines[nugget->occupied] = line;
    nugget->occupied++;
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Helper function around write_valuepool (values.c) to put constant values into the Value pool. write_valuepool 
 * increments the occupied value, so return (occupied - 1) which is the index of this particular value.
 */
int add_constant(Nugget* nugget, Value value) {
    write_valuepool(&nugget->constants, value);
    return (nugget->constants.occupied - 1);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Have a look at debug.c->constant_instruction. OPCODE_CONSTANT only uses a single byte for its operand (constant
 * location), which means that a nugget is restricted to storing and indexing only 256 unique constant values. This
 * is sufficiently small that any moderately-sized program could encounter that hard limit.
 * We'd like to keep the cache coherency and consistency that the single-byte OPCODE_CONSTANT provides, so write_constant()
 * implements a different opcode, OPCODE_CONSTANT_LONG, which stores the operand as a 24-bit number. Along with the opcode,
 * OPCODE_CONSTANT_LONG will therefore be an instruction 32 bits (4 bytes, for the nugget->code pointer) in total size.
 * The first byte is the opcode itself. If add_constant() returns a value below 255, then we are in the first 256 indices
 * of the valuepool and we can simply write the operand (the index byte) as-is.
 * If it's above this, then we need a larger index, which will be the remaining 24 bits in size. Write an OPCODE_LONG_CONSTANT
 * instead, and then pull out the first, second, and third bytes of the index and write them to the nugget's code separately.
 * During disassembly and running, this index will be stitched back together from these three bytes.
 */
void write_constant(Nugget* nugget, Value value, int line) {
    printf("\nAdding value %g (from line %d)", value, line);
    int at_index = add_constant(nugget, value);

    if (at_index <= 255) {
        // An hour of debugging to realize this should have been LOW_BYTE instead of HIGH_BYTE. Fucking hell, wine please.
        uint8_t operand = LOW_BYTE(at_index);
        write_nugget(nugget, OPCODE_CONSTANT, line);
        write_nugget(nugget, operand, line);
    } else {
        uint8_t high_byte = HMID_BYTE(at_index);
        uint8_t mid_byte  = LMID_BYTE(at_index);
        uint8_t low_byte  = LOW_BYTE(at_index);
        write_nugget(nugget, OPCODE_CONSTANT_LONG, line);
        write_nugget(nugget, high_byte, line);
        write_nugget(nugget, mid_byte, line);
        write_nugget(nugget, low_byte, line);
    }
}