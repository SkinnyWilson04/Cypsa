#ifndef cypsa_debug_h
    #define cypsa_debug_h

    #include "nugget.h"

    void disassemble_nugget(Nugget* nugget, const char* name);
    int disassemble_instruction(Nugget* nugget, int offset);
    
#endif