/* ast.h */

#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Forward declaration */
typedef struct ASTNode ASTNode;

typedef enum
{
    OP_PLUS,
    OP_MINUS,
    OP_TIMES,
    OP_DIVIDE,
    OP_MOD,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,
    OP_EQ,
    OP_NE,
    OP_AND,
    OP_OR,
    OP_NEG // For unary minus
    // Add other operators as needed
} OperatorType;

/* AST node types */
typedef enum
{
    NODE_NUMBER,
    NODE_CHAR,
    NODE_BOOLEAN,
    NODE_IDENTIFIER,
    NODE_ASSIGNMENT,
    NODE_OPERATION,
    NODE_UNARY_OPERATION,
    NODE_FOR_STATEMENT,
    NODE_WHILE_STATEMENT,
    NODE_PRINT_STATEMENT,
    NODE_ERROR_STATEMENT,
    NODE_STATEMENT_LIST,
    NODE_IF_STATEMENT,
    NODE_STRING_LITERAL,
    NODE_SWITCH_STATEMENT,
    NODE_CASE,
    NODE_DEFAULT_CASE,
    NODE_BREAK_STATEMENT,
    NODE_FUNC_CALL,
} NodeType;

/* Structures */
typedef struct StatementList
{
    ASTNode *statement;
    struct StatementList *next;
} StatementList;

typedef struct
{
    ASTNode *condition;
    ASTNode *then_branch;
    ASTNode *else_branch;
} IfStatementNode;

typedef struct CaseNode
{
    ASTNode *value;
    ASTNode *statements;
    struct CaseNode *next;
} CaseNode;

typedef struct
{
    ASTNode *expression; // The switch expression
    CaseNode *cases;     // Linked list of cases
} SwitchNode;

typedef struct ArgumentList
{
    struct ASTNode *expr;
    struct ArgumentList *next;
} ArgumentList;

typedef struct
{
    bool is_volatile;
    bool is_signed;
    bool is_unsigned;
    bool is_boolean;
} TypeModifiers;

/* AST node structure */
struct ASTNode
{
    NodeType type;
    TypeModifiers modifiers;
    union
    {
        int value;  // For numbers
        char *name; // For identifiers
        struct
        { // For binary operations and assignments
            ASTNode *left;
            ASTNode *right;
            OperatorType op; // Operator character like '+', '-', etc.
        } op;
        struct
        { // For unary operations
            ASTNode *operand;
            char op; // Operator character like '-', etc.
        } unary;
        struct
        { // For 'for' statements
            ASTNode *init;
            ASTNode *cond;
            ASTNode *incr;
            ASTNode *body;
        } for_stmt;
        struct
        { // For 'while' statements
            ASTNode *cond;
            ASTNode *body;
        } while_stmt;
        struct
        {
            char *function_name;
            ArgumentList *arguments;
        } func_call;
        StatementList *statements; // For statement lists
        IfStatementNode if_stmt;   // For if statements
        SwitchNode switch_stmt;
        CaseNode case_node;
        ASTNode *break_stmt; // For break statements, can be NULL
        // Add other nodes as needed
    } data;
};

/* Function prototypes */
ASTNode *create_number_node(int value);
ASTNode *create_char_node(char value);
ASTNode *create_boolean_node(int value);
ASTNode *create_identifier_node(char *name);
ASTNode *create_assignment_node(char *name, ASTNode *expr);
ASTNode *create_operation_node(OperatorType op, ASTNode *left, ASTNode *right);
ASTNode *create_unary_operation_node(OperatorType op, ASTNode *operand);
ASTNode *create_for_statement_node(ASTNode *init, ASTNode *cond, ASTNode *incr, ASTNode *body);
ASTNode *create_while_statement_node(ASTNode *cond, ASTNode *body);
ASTNode *create_function_call_node(char *func_name, ArgumentList *args);
ArgumentList *create_argument_list(ASTNode *expr, ArgumentList *existing_list);
ASTNode *create_print_statement_node(ASTNode *expr);
ASTNode *create_error_statement_node(ASTNode *expr);
ASTNode *create_statement_list(ASTNode *statement, ASTNode *next_statement);
ASTNode *create_if_statement_node(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch);
ASTNode *create_string_literal_node(char *string);
ASTNode *create_switch_statement_node(ASTNode *expression, CaseNode *cases);
CaseNode *create_case_node(ASTNode *value, ASTNode *statements);
CaseNode *create_default_case_node(ASTNode *statements);
CaseNode *append_case_list(CaseNode *list, CaseNode *case_node);
ASTNode *create_break_node();
TypeModifiers get_current_modifiers(void);

int evaluate_expression(ASTNode *node);
void execute_statement(ASTNode *node);
void execute_statements(ASTNode *node);
void execute_for_statement(ASTNode *node);
void execute_while_statement(ASTNode *node);
void execute_yapping_call(ArgumentList *args);
void execute_yappin_call(ArgumentList *args);
void execute_baka_call(ArgumentList *args);
void free_ast(ASTNode *node);
void reset_modifiers(void);

extern TypeModifiers current_modifiers;

#endif /* AST_H */
