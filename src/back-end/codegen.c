#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "codegen.h"
#include "../front-end/ast.h"

#define MAX_BUF_LEN 1024

/*reminder: printf code:

declare i32 @printf(i8* noalias nocapture, ...)
call i32 (i8*, ...)* @printf(i8* %msg, i32 12, i8 42)

*/

char* freeing[MAX_BUF_LEN];
int j = 0;
int var_c = 0;

char* types(char* t) {
    if (!strcmp(t, "int")) {
        return "i32";
    }
    return "";
}

void generate(Node** ast, int size, char* code) {
    strcat(code, "define i32 @main() {\n");
    for (int i = 0; i < size; i++) {
        Node* n = ast[i];
        if (n == NULL) {
            break;
        }
        if (n->n_type == VAR_DECLARATION_NODE) {
            Var_declaration_node* v = (Var_declaration_node*) n;
            char* name = generate_expression_asm(v->value, code);
            strcat(code, "%");
            strcat(code, v->name);
            strcat(code, " = ");
            strcat(code, name);
            strcat(code, "\n");
        }
    }
    strcat(code, "ret i32 0\n}");
    for (int i = 0; i < j; i++) {
        free(freeing[i]);
    }
}
