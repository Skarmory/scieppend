#include "scieppend/test/test.h"
#include "scieppend/test/core/array.h"
#include "scieppend/test/core/cache.h"
#include "scieppend/test/core/cache_map.h"
#include "scieppend/test/core/ecs.h"
#include "scieppend/test/core/event.h"
#include "scieppend/test/core/stack_array.h"
#include "scieppend/test/core/link_array.h"

int main(int argc, char** argv)
{
    test_init(true);

    test_array_run_all();
    test_stack_array_run_all();
    test_cache_run_all();
    test_linkarray_run_all();
    test_cache_map_run_all();
    test_ecs_run_all();
    test_event_run_all();

    testing_run_tests();
    testing_report();

    test_uninit();

    return 0;
}
