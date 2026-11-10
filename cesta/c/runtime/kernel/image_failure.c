#include "sexos_kernel.h"

_Noreturn void program_image_bounds_failure(void)
{
    kernel_halt("checked program-image byte access");
    for (;;) {}
}
