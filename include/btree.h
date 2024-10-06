#ifndef BTREE_H
#define BTREE_H

#include "row.h"
#include "table.h"
#include <stdbool.h>
#include <stdint.h>

#define MAX 3
#define MIN 2

typedef enum {
    NODE_INTERNAL,
    NODE_LEAF,
} NodeType;

typedef struct {
    uint32_t TYPE_SIZE;
    uint32_t TYPE_OFFSET;
    uint32_t IS_ROOT_SIZE;
    uint32_t IS_ROOT_OFFSET;
    uint32_t PARENT_POINTER_SIZE;
    uint32_t PARENT_POINTER_OFFSET;
    uint32_t TOTAL_SIZE;
} CommonNodeHeader;

typedef struct {
    uint32_t NUM_CELLS_SIZE;
    uint32_t NUM_CELLS_OFFSET;
    uint32_t TOTAL_SIZE;
} LeafNodeHeader;

typedef struct {
    uint32_t KEY_SIZE;
    uint32_t KEY_OFFSET;
    uint32_t VALUE_SIZE;
    uint32_t VALUE_OFFSET;
    uint32_t CELL_SIZE;
} LeafNodeBody;

// Node layout configuration
typedef struct {
    CommonNodeHeader common;
    LeafNodeHeader leaf;
    LeafNodeBody leafBody;
    uint32_t LEAF_NODE_SPACE_FOR_CELLS;
    uint32_t LEAF_NODE_MAX_CELLS;
    uint32_t LEAF_NODE_RIGHT_SPLIT_COUNT;
} NodeLayout;

NodeLayout InitNodeLayout();
NodeType GetNodeType(void*);
uint32_t* GetLeafNodeNumCells(void*, NodeLayout*);
void* GetLeafNodeCell(void*, NodeLayout*, uint32_t cellNum);
uint32_t* GetLeafNodeKey(void*, NodeLayout*, uint32_t cellNum);
uint32_t* GetLeafNodeValue(void*, NodeLayout*, uint32_t cellNum);
uint32_t FindLeafNodeIndex(void* node, uint32_t key);
void InitialiseLeafNode(void*, NodeLayout*);
void InsertToLeafNode(Cursor*, uint32_t key, Row*, const Field[]);
void SplitAndInsertToLeafNode(Cursor* cursor, NodeLayout* layout, uint32_t key, Row* value, const Field fields[]);
void SplitToRightInLeafNode(void* source, void* destination, NodeLayout* layout, uint32_t startIndex, uint32_t endIndex);
void InsertToLeafNodeAfterSplit(void* node, NodeLayout* layout, uint32_t key, Row* value, const Field fields[], uint32_t index);
void PrintLeafNode(void*);
void PrintConstants();

#endif /* ifndef BTREE_H */
