#ifndef AST_H
#define AST_H

#include "scanner.h"
#include "symbols.h"

#define MAX_NAME_LEN  100
#define MAX_TYPE_LEN  8
#define MAX_OPER_LEN 3

struct Node;

typedef enum {
    VAR_DECLARATION_NODE,
    VAR_ASSIGN_NODE,
    OPERATOR_NODE,
    INTEGER_NODE,
    BOOL_NODE,
    STRING_NODE,
    CHAR_NODE,
    REAL_NODE,
    ID_NODE,
    BLOCK_NODE,
    WRITE_NODE,
    READ_NODE,
    NODE_NODE,
    FUNC_CALL_NODE,
    FUNC_DECL_NODE,
    RET_NODE,
} NodeType;

typedef struct {
    NodeType n_type;
    char padding[1024];
} Node;

typedef struct {
    NodeType n_type;
    char type[MAX_TYPE_LEN];
    char name[MAX_NAME_LEN];
    char codegen_name[MAX_NAME_LEN + 4];
    Node* value;
} Var_declaration_node;

typedef struct {
    NodeType n_type;
    char name[MAX_NAME_LEN];
    Node* value;
    SymbolTable* currentScope;
    char codegen_name[MAX_NAME_LEN + 4];
} Var_assignment_node;

typedef struct {
    NodeType n_type;
    char name[MAX_NAME_LEN];
    Node** args;
    int args_len;
} Func_call_node;

typedef struct {
    NodeType n_type;
    char name[MAX_NAME_LEN];
    char codegen_name[MAX_NAME_LEN + 4];
    char type[MAX_TYPE_LEN];
    SymbolTable* currentScope;
} Identifier_node;

typedef struct {
    NodeType n_type;
    int value;
} Integer_node;


typedef struct {
    NodeType n_type;
    int value;
} Boolean_node;

typedef struct {
    NodeType n_type;
    double value;
} Real_node;

typedef struct {
    NodeType n_type;
    char value;
} Char_node;

typedef struct {
    NodeType n_type;
    char* value;
} String_node;

typedef struct {
    NodeType n_type;
    Node* left;
    Node* right;
    char oper[MAX_OPER_LEN];
} Operator_node;

typedef struct {
    NodeType n_type;
    Node** statements;
    int ssize;
} Block_node;

typedef struct {
    NodeType n_type;
    Node** args;
    Node* body;
    int args_len;
    char type[MAX_TYPE_LEN];
    char name[MAX_NAME_LEN];
} Func_decl_node;

typedef struct {
    NodeType n_type;
    Node* expr;
} Return_node;

void free_node(Node* n);
void print_node(FILE* f, Node* n);
Var_declaration_node* new_Var_declaration_node(char* name, char* codegen_name, char* type, Node* value);
Var_assignment_node* new_Var_assignment_node(char* name, Node* value, char* cgid);
Operator_node* new_Operator_node(char* oper, Node* left, Node* right);
Integer_node* new_Integer_node(int val);
Boolean_node* new_Boolean_node(int val);
String_node* new_String_node(char* val);
Char_node* new_Char_node(char val);
Real_node* new_Real_node(double val);
Identifier_node* new_Identifier_node(char* name, char* codegen_name, char* type);
Func_call_node* new_Func_call_node(char* name, Node** args);
Func_decl_node* new_Func_decl_node(char* name, char* type, Node** args, int args_len, Node* body);
Return_node* new_Return_node(Node* expr);
Block_node* new_Block_node(Node** statements, int ssize);

#endif
