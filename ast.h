#include "scanner.h"

#define MAX_NAME_LEN  128
#define MAX_TYPE_LEN  8
#define MAX_OPER_LEN 3

struct Node;

typedef enum {
    VAR_DECLARATION_NODE,
    OPERATOR_NODE,
    INTEGER_NODE,
    NODE_NODE,
} NodeType;

typedef struct {
    NodeType n_type;
    char padding[1024];
} Node;

typedef struct {
    NodeType n_type;
    char type[MAX_TYPE_LEN];
    char name[MAX_NAME_LEN];
    Node* value;
} Var_declaration_node;

typedef struct {
    NodeType n_type;
    int value;
} Integer_node;

typedef struct {
    NodeType n_type;
    Node* left;
    Node* right;
    char oper[MAX_OPER_LEN];
} Operator_node;

void free_node(Node* n);

