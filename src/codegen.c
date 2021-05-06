#include <stdio.h>
#include <string.h>
#include "ast.h"

void generate(Node** ast, char* code) {
    for (int i = 0; i < sizeof(ast) / sizeof(Node*); i++) {
        Node* cur_node = ast[i];
    }
}
