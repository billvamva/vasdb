#include "btree.h"
#include "pager.h"
#include "row.h"
#include "table.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

NodeLayout InitNodeLayout()
{
    NodeLayout layout = {
        .common = {
            .TYPE_SIZE = sizeof(uint8_t),
            .TYPE_OFFSET = 0,
            .IS_ROOT_SIZE = sizeof(uint8_t),
            .IS_ROOT_OFFSET = sizeof(uint8_t), // TYPE_SIZE
            .PARENT_POINTER_SIZE = sizeof(uint32_t),
            .PARENT_POINTER_OFFSET = 2 * sizeof(uint8_t), // IS_ROOT_OFFSET + IS_ROOT_SIZE
            .TOTAL_SIZE = 2 * sizeof(uint8_t) + sizeof(uint32_t) },
        .leaf = { .NUM_CELLS_SIZE = sizeof(uint32_t),
            .NUM_CELLS_OFFSET = 2 * sizeof(uint8_t) + sizeof(uint32_t), // COMMON_NODE_HEADER_SIZE
            // common header + leaf header size
            .TOTAL_SIZE = 2 * sizeof(uint8_t) + 2 * sizeof(uint32_t) },
        .leafBody = {
            .KEY_SIZE = sizeof(uint32_t), .KEY_OFFSET = 0, .VALUE_SIZE = sizeof(Row),
            .VALUE_OFFSET = sizeof(uint32_t), // KEY_SIZE
            .CELL_SIZE = sizeof(uint32_t) + sizeof(Row) // KEY_SIZE + VALUE_SIZE
        }
    };

    layout.LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - layout.leaf.TOTAL_SIZE;
    layout.LEAF_NODE_MAX_CELLS = layout.LEAF_NODE_SPACE_FOR_CELLS / layout.leafBody.CELL_SIZE;

    return layout;
}

NodeType GetNodeType(void* node)
{
    NodeLayout layout = InitNodeLayout();
    uint8_t* value = node + layout.common.TYPE_OFFSET;

    return (NodeType)(*value);
}

void SetNodeType(void* node, NodeType nodeType)
{
    NodeLayout layout = InitNodeLayout();
    *((uint8_t*)(node + layout.common.TYPE_OFFSET)) = (uint8_t)nodeType;
}

uint32_t* GetLeafNodeNumCells(void* node, NodeLayout* layout)
{
    return node + layout->leaf.NUM_CELLS_OFFSET;
}

void* GetLeafNodeCell(void* node, NodeLayout* layout, uint32_t cellNum)
{
    return node + layout->leaf.TOTAL_SIZE + cellNum * layout->leafBody.CELL_SIZE;
}

uint32_t* GetLeafNodeKey(void* node, NodeLayout* layout, uint32_t cellNum)
{
    return GetLeafNodeCell(node, layout, cellNum) + layout->leafBody.KEY_OFFSET;
}

uint32_t* GetLeafNodeValue(void* node, NodeLayout* layout, uint32_t cellNum)
{
    return GetLeafNodeCell(node, layout, cellNum) + layout->leafBody.VALUE_OFFSET;
}

/*
Sets to Node_leaf enum and num cells to 0
*/
void InitialiseLeafNode(void* node, NodeLayout* layout)
{
    SetNodeType(node, NODE_LEAF);
    *GetLeafNodeNumCells(node, layout) = 0;
}

/*
Finds the index for the specified key in the leaf node using binary search
*/
uint32_t FindLeafNodeIndex(void* node, uint32_t key)
{
    NodeLayout layout = InitNodeLayout();
    uint32_t numCells = *GetLeafNodeNumCells(node, &layout);

    // Handle empty node case
    if (numCells == 0) {
        return 0;
    }

    uint32_t lowIndex = 0;
    uint32_t highIndex = numCells - 1;

    while (lowIndex <= highIndex) {
        uint32_t currentIndex = lowIndex + (highIndex - lowIndex) / 2;
        uint32_t currentKey = *GetLeafNodeKey(node, &layout, currentIndex);

        if (currentKey == key) {
            return currentIndex;
        } else if (currentKey < key) {
            lowIndex = currentIndex + 1;
        } else {
            // Handle the case where currentIndex is 0
            if (currentIndex == 0) {
                return 0;
            }
            highIndex = currentIndex - 1;
        }
    }

    return lowIndex;
}

/*
Insert key value pair in the cursor position. Takes care of splitting if required
*/
void InsertToLeafNode(Cursor* cursor, uint32_t key, Row* value, const Field fields[])
{
    void* node = GetCursorPosition(cursor);

    NodeLayout layout = InitNodeLayout();
    uint32_t numCells = *GetLeafNodeNumCells(node, &layout);

    if (numCells >= layout.LEAF_NODE_MAX_CELLS) {
        SplitAndInsertToLeafNode(cursor, &layout, key, value, fields);
        return;
    }

    // Make room for new cell
    if (cursor->cellNum < numCells) {
        for (uint32_t i = numCells; i > cursor->cellNum; i--) {
            void* destination = GetLeafNodeCell(node, &layout, i);
            void* source = GetLeafNodeCell(node, &layout, i - 1);
            memcpy(destination, source, layout.leafBody.CELL_SIZE);
        }
    }

    // Insert new cell
    *(GetLeafNodeNumCells(node, &layout)) += 1;
    *(GetLeafNodeKey(node, &layout, cursor->cellNum)) = key;
    SerialiseRow(value, GetLeafNodeValue(node, &layout, cursor->cellNum), fields);
}

// Adds the value in the array, splits into two, and moves 1 to the parent(or creates parent)
void SplitAndInsertToLeafNode(Cursor* cursor, NodeLayout* layout, uint32_t key, Row* value, const Field fields[])
{
    void* oldPage = GetPage(cursor->table->pager, cursor->pageNum);

    uint32_t newPageNum = GetUnusedPageNum(cursor->table->pager);
    void* newPage = GetPage(cursor->table->pager, newPageNum);

    InitialiseLeafNode(newPage, layout);

    uint32_t oldNumCells = *GetLeafNodeNumCells(oldPage, layout);
    uint32_t insertIndex = FindLeafNodeIndex(oldPage, key);
    uint32_t splitIndex = (layout->LEAF_NODE_MAX_CELLS + 1) / 2;

    SplitToRightInLeafNode(oldPage, newPage, layout, splitIndex, oldNumCells - 1);
    *(GetLeafNodeNumCells(oldPage, layout)) = layout->LEAF_NODE_MAX_CELLS - splitIndex;
    *(GetLeafNodeNumCells(newPage, layout)) = splitIndex;

    // new key will be in the left side of the tree
    if (splitIndex > insertIndex) {
        InsertToLeafNodeAfterSplit(oldPage, layout, key, value, fields, insertIndex);
    } else {

        InsertToLeafNodeAfterSplit(newPage, layout, key, value, fields, insertIndex);
    }
}

// will move right split to destination node
void SplitToRightInLeafNode(void* source, void* destination, NodeLayout* layout, uint32_t startIndex, uint32_t endIndex)
{
    for (uint32_t i = startIndex; i <= endIndex; i--) {
        void* sourceCell = GetLeafNodeCell(source, layout, i);
        void* destinationCell = GetLeafNodeCell(destination, layout, startIndex - i);
        memcpy(destinationCell, sourceCell, layout->leafBody.CELL_SIZE);
    }
}

/*
Inserts to leaf node after split
*/
void InsertToLeafNodeAfterSplit(void* node, NodeLayout* layout, uint32_t key, Row* value, const Field fields[], uint32_t index)
{
    uint32_t numCells = *GetLeafNodeNumCells(node, layout);
    // move cells if needed
    for (uint32_t i = numCells; i > index; i--) {
        void* destination = GetLeafNodeCell(node, layout, i);
        void* source = GetLeafNodeCell(node, layout, i - 1);
        memcpy(destination, source, layout->leafBody.CELL_SIZE);
    }

    // Insert new cell
    *(GetLeafNodeKey(node, layout, index)) = key;
    SerialiseRow(value, GetLeafNodeValue(node, layout, index), fields);

    // increment cell count
    *(GetLeafNodeNumCells(node, layout)) += 1;
}

void PrintLeafNode(void* node)
{
    NodeLayout layout = InitNodeLayout();
    uint32_t numCells = *GetLeafNodeNumCells(node, &layout);

    printf("leaf of size: %d \n", numCells);

    for (uint32_t i = 0; i < numCells; i++) {
        uint32_t key = *GetLeafNodeKey(node, &layout, i);
        printf("  - %d : %d\n", i, key);
    }
}

void PrintConstants()
{
    NodeLayout layout = InitNodeLayout();
    printf("ROW_SIZE: %lu\n", sizeof(Row));
    printf("COMMON_NODE_HEADER_SIZE: %d\n", layout.common.TOTAL_SIZE);
    printf("LEAF_NODE_HEADER_SIZE: %d\n", layout.leaf.TOTAL_SIZE);
    printf("LEAF_NODE_CELL_SIZE: %d\n", layout.leafBody.CELL_SIZE);
    printf("LEAF_NODE_SPACE_FOR_CELLS: %d\n", layout.LEAF_NODE_SPACE_FOR_CELLS);
    printf("LEAF_NODE_MAX_CELLS: %d\n", layout.LEAF_NODE_MAX_CELLS);
}
