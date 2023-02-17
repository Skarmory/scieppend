#include "scieppend/test/test.h"
#include "scieppend/test/core/array.h"
#include "scieppend/test/core/stack_array.h"

int main(int argc, char** argv)
{
    test_init();
    test_array_run_all();
    test_stack_array_run_all();
    test_uninit();

    return 0;
}
