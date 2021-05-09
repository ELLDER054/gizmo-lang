#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "codegen.h"
#include "../front-end/ast.h"

int str_append_c = 0;

char* generate_oper_asm(char* oper, Node* left, Node* right) {
    char ret[1024];
    switch (*oper) {
        case '+':
            str_append_c += snprintf(ret + str_append_c, 100, "add %s, %s", (gnerate_expression_asm(left), gnerate_expression_asm(right));
            return ret;
        case '-':
            str_append_c += snprintf(ret + str_append_c, 100, "sub %s, %s", (gnerate_expression_asm(left), gnerate_expression_asm(right));
            return ret;
        case '*':
            str_append_c += snprintf(ret + str_append_c, 100, "mul %s, %s", (gnerate_expression_asm(left), gnerate_expression_asm(right));
            return ret;
        case '/':
            str_append_c += snprintf(ret + str_append_c, 100, "div %s, %s", (gnerate_expression_asm(left), gnerate_expression_asm(right));
            return ret;
        default:
            return '_';
    }
}

char* generate_expression_asm(Node* n) {
    char code[1024];
    if (n->n_type == INTEGER_NODE) {
        str_append_c += snprintf(code + str_append_c, 100, "%d", ((Integer_node*) n)->value);
        return code;
    } else if (n->n_type == ID_NODE) {
        str_append_c += snprintf(code + str_append_c, 100, "%%%s", ((Identifier_node*) n)->name);
        return code;
    }
    
    return generate_oper_asm(((Operator_node*) n)->oper, ((Operator_node*) n)->left, ((Operator_node*) n)->right);
}

void generate(Node** ast, int size, char* code) {
    for (int i = 0; i < size; i++) {
        Node* n = ast[i];
        if (n == NULL) {
            break;
        }
        if (n->n_type == VAR_DECLARATION_NODE) {
            Var_declaration_node* v = (Var_declaration_node*) n;
            str_append_c += snprintf(code + str_append_c, 100, "%%%s = %s", v->name, generate_expression_asm(v->value));
            printf("%s\n", code);
        }
    }
}
