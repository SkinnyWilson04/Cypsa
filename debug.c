#include <stdio.h>
#include "debug.h"
#include "values.h"


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * The disassemble_nugget() and _instruction() functions provide some primitive debugging of a given
 * code nugget, simply displaying the opcodes corresponding to the values in the *code array.
 * disassemble_nugget() first prints a header, then begins looping through the instructions in the
 * *code array.
 * NOTE that the for-loop does not increment the offset, as instructions can be more than a single
 * byte in size. Updating the offset is handled in disassemble_instruction() which will switch on
 * the instruction given and return the correct corresponding byte size.
 */
void disassemble_nugget(Nugget* nugget, const char* op_name) {
    printf("\n* ~ ~ ~ ~ ~ ~ %s ~ ~ ~ ~ ~ ~ *\n", op_name);

    for (int offset = 0; offset < nugget->occupied; /* No increment */ ) {
        offset = disassemble_instruction(nugget, offset);
    }
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * OPCODE_LONG_CONSTANT constant values are obtained with 24-bit index values. This is serialized in the
 * nugget.code[] block as 4 sequential bytes - the opcode followed by the high, middle, and low bytes.
 * To index the actual Value, this index needs to be reconstructed by shifting the bytes into the correct
 * places and then OR-ing everything together.
 */
static int reconstruct_long_location(uint8_t high, uint8_t mid, uint8_t low) {
    return ((high << 16) | (mid << 8) | (low));
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Simple instructions are just single-byte instructs with no other parameters we need to handle. For
 * these, we can just print the corresponding name, increment the offset, and return.
 */
static int simple_instruction(const char* op_name, int offset) {
    printf("%s\n", op_name);
    return (offset + 1);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Handles instructions which load some constant / immediate value. OPCODE_CONSTANT is a two-byte instruction;
 * one byte for the opcode, and one for the operand (location of the Value). First, the location of the operand
 * is taken from the next byte along in the code [offset + 1], and then the Value is loaded from the ValuePool
 * value array. First, the opcode name and index of the constant are printed. The value is handed off to the
 * print_value() helper (values.c) for display. Since this is a two-byte instruction, offset is incremented by 2
 */
static int constant_instruction(const char* op_name, Nugget* nugget, int offset) {
    uint8_t constant_location = nugget->code[offset + 1];
    Value constant_value      = nugget->constants.values[constant_location];
    printf("%-16s [%4d]  ", op_name, constant_location);
    print_value(constant_value);
    return (offset + 2);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * OPCODE_CONSTANT_LONG is a 4-byte instruction:
 *      [opcode, high, mid, low]
 * Where high, mid, and low are the byte values which constitute a 24-bit index into the constants.values[] array.
 * This longer 24-bit index greatly expands the number of constants that can be addressed beyond the limit of 256
 * using the 1-byte index of plain OPCODE_CONSTANT. To reconstruct this the three bytes after the opcode (high, mid,
 * and low bytes of the final index) are taken from the code array and passed to reconstruct_long_location(), which
 * shifts the values into the correct locations and ORs them together. The resulting 32-bit int is used to index 
 * the actual constant value, which is displayed.
 */
static int constant_long_instruction(const char* op_name, Nugget* nugget, int offset) {
    uint8_t high_byte     = nugget->code[offset + 1];
    uint8_t mid_byte      = nugget->code[offset + 2];
    uint8_t low_byte      = nugget->code[offset + 3];
    int constant_location = reconstruct_long_location(high_byte, mid_byte, low_byte);
    Value constant_value  = nugget->constants.values[constant_location];

    printf("%-16s [%4d]  ", op_name, constant_location);
    print_value(constant_value);
    return (offset + 4);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Basically just a big switch statement which determines the type of the instruction at the current offset, and
 * hands off a name and its operand(s) to an appropriate display / logging function. If we hit the default (an
 * unknown instruction), just increment the offset by 1 and try again. 
 * The initial if checks whether the line in the source code that the current instruction came from is the same
 * as the previous. If instructions come from the same line of source code, then indent and print a | to visually
 * indicate this. Else, just print the line number.
 */
int disassemble_instruction(Nugget* nugget, int offset) {
    printf("%04d\t->\t", offset);

    if (offset > 0 && nugget->lines[offset] == nugget->lines[offset - 1]) {
        printf("     |> ");
    } else {
        printf("%4d ", nugget->lines[offset]);
    }

    uint8_t instruction = nugget->code[offset];

    switch (instruction) {
        case OPCODE_CONSTANT:
            return constant_instruction("OPCODE_CONSTANT", nugget, offset);
        case OPCODE_CONSTANT_LONG:
            return constant_long_instruction("OPCODE_CONSTANT_LONG", nugget, offset);
        case OPCODE_NEGATE:
            return simple_instruction("OPCODE_NEGATE", offset);
        case OPCODE_ADD:
            return simple_instruction("OPCODE_ADD", offset);
        case OPCODE_SUBTRACT:
            return simple_instruction("OPCODE_SUBTRACT", offset);
        case OPCODE_MULTIPLY:
            return simple_instruction("OPCODE_MULTIPLY", offset);
        case OPCODE_DIVIDE:
            return simple_instruction("OPCODE_DIVIDE", offset);
        case OPCODE_RETURN:
            return simple_instruction("OPCODE_RETURN", offset);
        default:
            printf("Encountered unknown / unimplemented Opcode '%d' [offset: %04d]", instruction, offset);
            return offset + 1;
    }
}
