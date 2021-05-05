#ifndef AST_H
#define AST_H

#include "scanner.h"

#define MAX_NAME_LEN  128
#define MAX_TYPE_LEN  8
#define MAX_OPER_LEN 3

struct Node;

typedef enum {
    VAR_DECLARATION_NODE,
    OPERATOR_NODE,
    INTEGER_NODE,
    STRING_NODE,
    REAL_NODE,
    ID_NODE,
    NODE_NODE,
} NodeType;

typedef enum {
    INT_TYPE,
    STRING_TYPE,
    CHAR_TYPE,
    BOOL_TYPE,
    REAL_TYPE,
} Type;

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
    char* name;
} Identifier_node;

typedef struct {
    NodeType n_type;
    int value;
    Type type;
} Integer_node;

typedef struct {
    NodeType n_type;
    double value;
    Type type;
} Real_node;

typedef struct {
    NodeType n_type;
    char* value;
    Type type;
} String_node;

typedef struct {
    NodeType n_type;
    Node* left;
    Node* right;
    char oper[MAX_OPER_LEN];
    Type type;
} Operator_node;

void free_node(Node* n);
Var_declaration_node* new_Var_declaration_node(char* name, char* type, Node* value);
Operator_node* new_Operator_node(char* oper, Node* left, Node* right);
Integer_node* new_Integer_node(int val);
String_node* new_String_node(char* val);
Real_node* new_Real_node(double val);
Identifier_node new_Id_node(char* name);

#endif
