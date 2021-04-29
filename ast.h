#include "scanner.h"

struct Node;

typedef enum {
    attr1,
    attr2,
    attr3,
    attr4,
} NodeType;

typedef struct {
    NodeType node;
} Node;

typedef struct {
    NodeType node;
    TokenType type;
    char name[126];
    char* value;
} Var_declaration_node;

typedef struct {
    NodeType node;
    int value;
} Integer_node;

typedef struct {
    NodeType node;
    Node left;
    Node right;
    char oper[3];
} Operator_node;
