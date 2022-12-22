#include <stdio.h>
#include "memory.h"
#include "values.h"


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Mirrors init_nugget() (nugget.c) - zero out the capacity and occupied count, and set the pointer to NULL
 */
void init_valuepool(ValuePool* pool) {
    pool->capacity = 0;
    pool->occupied = 0;
    pool->values   = NULL;
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Mirrors the code for adding bytecode to nuggets (write_nugget(), nugget.c). Check if the array of values
 * is at maximum capacity and grow it if true. Write the value to the next slot in the value pool.
 */
void write_valuepool(ValuePool* pool, Value value) {
    if (pool->capacity < pool->occupied + 1) {
        int prev_capacity = pool->capacity;
        pool->capacity    = GROW_CAPACITY(prev_capacity);
        pool->values      = GROW_ARRAY(Value, pool->values, prev_capacity, pool->capacity);
    }

    pool->values[pool->occupied] = value;
    pool->occupied++;
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Remove all values from the value pool. Set the capacity to 0 and array to NULL using init_valuepool()
 */
void free_valuepool(ValuePool* pool) {
    FREE_ARRAY(Value, pool->values, pool->capacity);
    init_valuepool(pool);
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Prints out one of Cypsa's Value types. %g prints either in exponential scientific format (like %e), or in
 * normal decimal format (like %f, up to 6 decimal places) depending upon which is the shorter representation.
 * NOTE: I might just change this to %f or %lf because usually I'm not really keen on exponential formatting.
 */
void print_value(Value value) {
    printf("%g", value);
}