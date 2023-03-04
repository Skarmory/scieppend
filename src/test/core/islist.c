#include "scieppend/test/core/islist.h"

#include "scieppend/core/islist.h"

#include <stdio.h>

void test_islist_add(void)
{
    struct ISList list;
    islist_init(&list, sizeof(int), 8);

    for(int i = 0; i < 8; ++i)
    {
        int z = i * 7;
        islist_add(&list, &z);
    }

    for(int i = 0; i < 8; ++i)
    {
        int j = (int)islist_pop_front(&list);
        printf("%d\n", j);
    }

    islist_uninit(&list);
}

void test_islist_run_all(void)
{
    test_islist_add();
}
