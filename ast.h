#ifndef AST_H
#define AST_H

#include "lexer.h"
#include "symbol.h"

#define MAX_NAME_LEN  100
#define MAX_TYPE_LEN  100
#define MAX_OPER_LEN 3

struct Node;

typedef enum {
    VAR_DECLARATION_NODE,
    VAR_ASSIGN_NODE,
    WHILE_NODE,
    IF_NODE,
    SKIP_NODE,
    OPERATOR_NODE,
    INTEGER_NODE,
    ARRAY_NODE,
    INDEX_NODE,
    BOOL_NODE,
    STRING_NODE,
    CHAR_NODE,
    REAL_NODE,
    ID_NODE,
    BLOCK_NODE,
    WRITE_NODE,
    LEN_NODE,
    READ_NODE,
    APPEND_NODE,
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
    char* type;
    char* name;
    char* codegen_name;
    Node* value;
} Var_declaration_node;

typedef struct {
    NodeType n_type;
    char* name;
    Node* value;
    char* codegen_name;
} Var_assignment_node;

typedef struct {
    NodeType n_type;
    char* type; // The type of the array
    Node** elements; // Elements in the array
    int len; // Length of the array
} Array_node;

typedef struct {
    NodeType n_type;
    Node* id; // The identifier that comes before the square brackets
    Node* expr; // The expression inside the square brackets
    char* type; // Type of each element in the array or string
    char* cgid; // Code generation name of the object being indexed
} Index_node;

typedef struct {
    NodeType n_type;
    Node* condition; // Condition of the while loop
    Node* body; // Body of the while loop
    char* begin_cgid; // Code generation names saved for llvm
    char* end_cgid;
} While_loop_node;

typedef struct {
    NodeType n_type;
    Node* condition; // Condition of the if statement
    Node* body; // Body of the if statement
    Node* else_body; // Body of the else (NULL if else is not used)
    char* begin_cgid; // Code generation names saved for llvm
    char* else_cgid;
    char* end_cgid;
} If_node;

typedef struct {
    NodeType n_type;
    char* name; // Name of the function
    char* type; // Type of the function
    Node** args; // Arguments of the function
    int args_len; // Length of the functions arguments
} Func_call_node;

typedef struct {
    NodeType n_type;
    char* name; // Identifier's name
    char* codegen_name; // Identifier's code generation name
    char* type; // Identifier's type
} Identifier_node;

typedef struct {
    NodeType n_type;
    int value; // Literal integer
} Integer_node;

typedef struct {
    NodeType n_type;
    int skip_kind; // Either 0 or 1, standing for break/continue
    char* code; // llvm code to break or continue
} Skip_node;

typedef struct {
    NodeType n_type;
    int value; // Either 0 or 1, standing for true/false
} Boolean_node;

typedef struct {
    NodeType n_type;
    double value; // Literal decimal
} Real_node;

typedef struct {
    NodeType n_type;
    char value; // Literal character
} Char_node;

typedef struct {
    NodeType n_type;
    char* value; // Literal string
} String_node;

typedef struct {
    NodeType n_type;
    Node* left; // Left side of the expression
    Node* right; // Right side of the expression
    char* oper; // The operator used
} Operator_node;

typedef struct {
    NodeType n_type;
    Node** statements; // The array of statements in the block
    int ssize; // Length of statements
} Block_node;

typedef struct {
    NodeType n_type;
    Node** args; // Arguments of the function
    Node* body; // Body of the function
    int args_len; // Length of the function arguments
    char* type; // Type of the function
    char* name; // Name of the function
} Func_decl_node;

typedef struct {
    NodeType n_type;
    Node* expr; // Returned expression
} Return_node;

void free_node(Node* n);
void print_node(FILE* f, Node* n);
Var_declaration_node* new_Var_declaration_node(char* name, char* codegen_name, char* type, Node* value);
Var_assignment_node* new_Var_assignment_node(char* name, Node* value, char* cgid);
While_loop_node* new_While_loop_node(Node* condition, Node* body, char* bcgid, char* ecgid);
Skip_node* new_Skip_node(int kind, char* code);
If_node* new_If_node(Node* condition, Node* body, Node* else_body, char* bcgid, char* ecgid, char* elcgid);
Operator_node* new_Operator_node(char* oper, Node* left, Node* right);
Integer_node* new_Integer_node(int val);
Array_node* new_Array_node(char* type, Node** elements);
Index_node* new_Index_node(Node* id, Node* expr, char* type, char* cgid);
Boolean_node* new_Boolean_node(int val);
String_node* new_String_node(char* val);
Char_node* new_Char_node(char val);
Real_node* new_Real_node(double val);
Identifier_node* new_Identifier_node(char* name, char* codegen_name, char* type);
Func_call_node* new_Func_call_node(char* name, char* type, Node** args);
Func_decl_node* new_Func_decl_node(char* name, char* type, Node** args, int args_len, Node* body);
Return_node* new_Return_node(Node* expr);
Block_node* new_Block_node(Node** statements, int ssize);

#endif
