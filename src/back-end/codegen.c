#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "codegen.h"
#include "../front-end/ast.h"

char* needs_freeing[1024];
int j = 0;

void generate_expression(Node* v, char* code);

int str_append(char *target, size_t targetSize, const char * restrict format, ...) {
  va_list args;
  char temp[targetSize];
  int result;

  va_start(args, format);
  result = vsnprintf(temp, targetSize, format, args);
  if (result != EOF)
  {
    if (strlen(temp) + strlen(target) > targetSize)
    {
      fprintf(stderr, "appendToStr: target buffer not large enough to hold additional string");
      return 0;
    }
    strcat(target, temp);
  }
  va_end(args);
  return result;
}

char* generate_oper_asm(char* oper, Node* left, Node* right) {
    char* l = malloc(sizeof(char*));
    char* r = malloc(sizeof(char*));
    needs_freeing[j++] = l;
    needs_freeing[j++] = r;
    generate_expression(left, l);
    generate_expression(right, r);
    char* ret = malloc(sizeof(char*));
    needs_freeing[j++] = ret;
    switch (*oper) {
        case '+':
            str_append(ret, 6 + sizeof(l) + sizeof(r), "add %s, %s", l, r);
            return ret;
        case '-':
            str_append(ret, 6 + sizeof(l) + sizeof(r), "sub %s, %s", l, r);
            return ret;
        case '*':
            str_append(ret, 6 + sizeof(l) + sizeof(r), "mul %s, %s", l, r);
            return ret;
        case '/':
            str_append(ret, 6 + sizeof(l) + sizeof(r), "div %s, %s", l, r);
            return ret;
        default:
            return NULL;
    }
}

void generate_expression(Node* v, char* code) {
    if (v->n_type == INTEGER_NODE) {
        str_append(code, 100, "%d", ((Integer_node*) v)->value);
        return;
    } else if (v->n_type == ID_NODE) {
        str_append(code, 102, "%%%s", ((Identifier_node*) v)->name);
        return;
    }
    
    char* oper_asm = generate_oper_asm(((Operator_node*) v)->oper, ((Operator_node*) v)->left, ((Operator_node*) v)->right);
    str_append(code, sizeof(oper_asm), "%s", oper_asm);
}

void generate(Node** ast, int size, char* code) {
    for (int i = 0; i < size; i++) {
        Node* n = ast[i];
        if (n == NULL) {
            break;
        }
        if (n->n_type == VAR_DECLARATION_NODE) {
            Var_declaration_node* v = (Var_declaration_node*) n;
            char additional_code[1024] = "%";
            str_append(additional_code, sizeof(v->name), "%s", v->name);
            str_append(additional_code, 3, " = ");
            printf("a_code is: %s\n", additional_code);
            generate_expression(v->value, additional_code);
            printf("code is %s\n", additional_code);
            str_append(code, sizeof(additional_code), "%s", additional_code);
        }
    }
    for (int i = 0; i < sizeof(needs_freeing) / sizeof(char*); i++) {
        free(needs_freeing[i]);
    }
}
