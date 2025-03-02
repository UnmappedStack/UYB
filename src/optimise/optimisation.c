#include <optimisation.h>

/* Takes a pointer to an array of Function structures and the number of functions in the IR.
 * Changes the statements in the given function to be more optimised. */
void optimise(Function *IR, size_t num_functions) {
    /* TODO: Change which optimisations are done based on -OX flag.
     * Planned optimisations:
     *  - Folding [DONE]
     *  - Copy elimination [DONE]
     *  - Unused label removal
     *  - Function inlining
     *  - Loop unravelling(?) */
    opt_fold(IR, num_functions);
    opt_copy_elim(IR, num_functions);
}
