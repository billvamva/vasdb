#include "minunit.h" // Assuming you're using minunit for testing
#include <btree.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to create a test leaf node
void* create_test_leaf_node(uint32_t* keys, uint32_t num_keys)
{
    NodeLayout layout = InitNodeLayout();
    void* node = malloc(PAGE_SIZE);
    memset(node, 0, PAGE_SIZE);
    *GetLeafNodeNumCells(node, &layout) = num_keys;
    for (uint32_t i = 0; i < num_keys; i++) {
        *GetLeafNodeKey(node, &layout, i) = keys[i];
    }
    return node;
}

static char* test_find_existing_key()
{
    uint32_t keys[] = { 1, 3, 5, 7, 9 };
    void* node = create_test_leaf_node(keys, 5);

    mu_assert("Error: Didn't find existing key 1", FindLeafNodeIndex(node, 1) == 0);
    mu_assert("Error: Didn't find existing key 5", FindLeafNodeIndex(node, 5) == 2);
    mu_assert("Error: Didn't find existing key 9", FindLeafNodeIndex(node, 9) == 4);

    free(node);
    return 0;
}

static char* test_find_insertion_point()
{
    uint32_t keys[] = { 1, 3, 5, 7, 9 };
    void* node = create_test_leaf_node(keys, 5);

    mu_assert("Error: Wrong insertion point for key 0", FindLeafNodeIndex(node, 0) == 0);
    mu_assert("Error: Wrong insertion point for key 2", FindLeafNodeIndex(node, 2) == 1);
    mu_assert("Error: Wrong insertion point for key 6", FindLeafNodeIndex(node, 6) == 3);
    mu_assert("Error: Wrong insertion point for key 10", FindLeafNodeIndex(node, 10) == 5);

    free(node);
    return 0;
}

static char* test_empty_node()
{
    void* node = create_test_leaf_node(NULL, 0);

    mu_assert("Error: Wrong index for empty node", FindLeafNodeIndex(node, 5) == 0);

    free(node);
    return 0;
}

static char* test_single_key_node()
{
    uint32_t keys[] = { 5 };
    void* node = create_test_leaf_node(keys, 1);

    mu_assert("Error: Wrong index for key less than single key", FindLeafNodeIndex(node, 3) == 0);
    mu_assert("Error: Didn't find existing single key", FindLeafNodeIndex(node, 5) == 0);
    mu_assert("Error: Wrong index for key greater than single key", FindLeafNodeIndex(node, 7) == 1);

    free(node);
    return 0;
}

static char* test_duplicate_keys()
{
    uint32_t keys[] = { 1, 3, 5 };
    void* node = create_test_leaf_node(keys, 3);

    uint32_t index = FindLeafNodeIndex(node, 3);
    mu_assert("Error: Wrong index for first duplicate key", index == 1);

    free(node);
    return 0;
}

char* run_find_leaf_node_index_tests()
{
    mu_run_test(test_find_existing_key);
    mu_run_test(test_find_insertion_point);
    mu_run_test(test_empty_node);
    mu_run_test(test_single_key_node);
    mu_run_test(test_duplicate_keys);
    return 0;
}
