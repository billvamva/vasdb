#include "btree.h"
#include "common.h"
#include "minunit.h"
#include <stdlib.h>
#include <string.h>

// Test initialization of NodeLayout
static char* test_init_node_layout()
{
    NodeLayout layout = InitNodeLayout();
    mu_assert("error, LEAF_NODE_MAX_CELLS not calculated correctly",
        layout.LEAF_NODE_MAX_CELLS > 0 && layout.LEAF_NODE_MAX_CELLS < PAGE_SIZE);
    mu_assert("error, LEAF_NODE_SPACE_FOR_CELLS not calculated correctly",
        layout.LEAF_NODE_SPACE_FOR_CELLS > 0 && layout.LEAF_NODE_SPACE_FOR_CELLS < PAGE_SIZE);
    return 0;
}

// Test leaf node initialization
static char* test_initialize_leaf_node()
{
    NodeLayout layout = InitNodeLayout();
    void* node = malloc(PAGE_SIZE);
    InitialiseLeafNode(node, &layout);
    mu_assert("error, leaf node not initialized with 0 cells",
        *GetLeafNodeNumCells(node, &layout) == 0);
    free(node);
    return 0;
}

// Test getting and setting leaf node cells
static char* test_leaf_node_cells()
{
    NodeLayout layout = InitNodeLayout();
    void* node = malloc(PAGE_SIZE);
    InitialiseLeafNode(node, &layout);

    // Test setting and getting the number of cells
    *GetLeafNodeNumCells(node, &layout) = 1;
    mu_assert("error, number of cells not set correctly",
        *GetLeafNodeNumCells(node, &layout) == 1);

    // Test getting cell, key, and value pointers
    void* cell = GetLeafNodeCell(node, &layout, 0);
    uint32_t* key = GetLeafNodeKey(node, &layout, 0);
    void* value = GetLeafNodeValue(node, &layout, 0);

    mu_assert("error, cell pointer incorrect",
        (char*)cell == (char*)node + layout.leaf.TOTAL_SIZE);
    mu_assert("error, key pointer incorrect",
        (char*)key == (char*)cell + layout.leafBody.KEY_OFFSET);
    mu_assert("error, value pointer incorrect",
        (char*)value == (char*)cell + layout.leafBody.VALUE_OFFSET);

    // Test setting and getting a key-value pair
    *key = 42;
    strcpy(value, "test value");
    mu_assert("error, key not set correctly", *GetLeafNodeKey(node, &layout, 0) == 42);
    mu_assert("error, value not set correctly",
        strcmp((char*)GetLeafNodeValue(node, &layout, 0), "test value") == 0);

    free(node);
    return 0;
}

// Run all tests
char* run_leaf_node_tests()
{
    mu_run_test(test_init_node_layout);
    mu_run_test(test_initialize_leaf_node);
    mu_run_test(test_leaf_node_cells);
    return 0;
}
