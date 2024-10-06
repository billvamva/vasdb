#include <minunit.h>
#include <stdio.h>

extern char* run_input_buffer_tests();
extern char* run_compiler_tests();
extern char* run_leaf_node_tests();
extern char* run_pager_tests();
extern char* run_find_leaf_node_index_tests();

int tests_run = 0;

static char* all_tests()
{
    mu_run_test(run_input_buffer_tests);
    mu_run_test(run_compiler_tests);
    mu_run_test(run_leaf_node_tests);
    mu_run_test(run_pager_tests);
    mu_run_test(run_find_leaf_node_index_tests);
    return 0;
}

int main()
{
    char* result = all_tests();
    printf("Tests run: %d\n", tests_run);
    printf(result != 0 ? "FAILED\n" : "ALL TESTS PASSED\n");
    return result != 0;
}
