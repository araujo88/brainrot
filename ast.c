/* ast.c */

#include "ast.h"
#include <stdbool.h>
#include <setjmp.h>
#include <string.h>

static jmp_buf break_env;

void execute_switch_statement(ASTNode *node)
{
    int switch_value = evaluate_expression(node->data.switch_stmt.expression);
    CaseNode *current_case = node->data.switch_stmt.cases;
    int matched = 0;

    if (setjmp(break_env) == 0)
    {
        while (current_case)
        {
            if (current_case->value)
            {
                int case_value = evaluate_expression(current_case->value);
                if (case_value == switch_value || matched)
                {
                    matched = 1;
                    execute_statements(current_case->statements);
                }
            }
            else
            {
                // Default case
                if (matched || !matched)
                {
                    execute_statements(current_case->statements);
                    break;
                }
            }
            current_case = current_case->next;
        }
    }
    else
    {
        // Break encountered; do nothing
    }
}

/* Include the symbol table functions */
extern bool set_variable(char *name, int value);
extern int get_variable(char *name);
extern void yyerror(const char *s);
extern void yapping(const char *format, ...);
extern void yappin(const char *format, ...);
extern void baka(const char *format, ...);

/* Function implementations */

ASTNode *create_number_node(int value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_NUMBER;
    node->data.value = value;
    return node;
}

ASTNode *create_char_node(char value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_CHAR;
    node->data.value = value;
    return node;
}

ASTNode *create_identifier_node(char *name)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_IDENTIFIER;
    node->data.name = strdup(name);
    return node;
}

ASTNode *create_assignment_node(char *name, ASTNode *expr)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_ASSIGNMENT;
    node->data.op.left = create_identifier_node(name);
    node->data.op.right = expr;
    node->data.op.op = '=';
    return node;
}

ASTNode *create_operation_node(OperatorType op, ASTNode *left, ASTNode *right)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_OPERATION;
    node->data.op.left = left;
    node->data.op.right = right;
    node->data.op.op = op;
    return node;
}

ASTNode *create_unary_operation_node(OperatorType op, ASTNode *operand)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_UNARY_OPERATION;
    node->data.unary.operand = operand;
    node->data.unary.op = op;
    return node;
}

ASTNode *create_for_statement_node(ASTNode *init, ASTNode *cond, ASTNode *incr, ASTNode *body)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_FOR_STATEMENT;
    node->data.for_stmt.init = init;
    node->data.for_stmt.cond = cond;
    node->data.for_stmt.incr = incr;
    node->data.for_stmt.body = body;
    return node;
}

ASTNode *create_while_statement_node(ASTNode *cond, ASTNode *body)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_WHILE_STATEMENT;
    node->data.while_stmt.cond = cond;
    node->data.while_stmt.body = body;
    return node;
}

ASTNode *create_function_call_node(char *func_name, ArgumentList *args)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_FUNC_CALL;
    node->data.func_call.function_name = strdup(func_name);
    node->data.func_call.arguments = args;
    return node;
}

ArgumentList *create_argument_list(ASTNode *expr, ArgumentList *existing_list)
{
    ArgumentList *new_node = malloc(sizeof(ArgumentList));
    new_node->expr = expr;
    new_node->next = NULL;

    if (!existing_list)
    {
        return new_node;
    }
    else
    {
        /* Append to the end of existing_list */
        ArgumentList *temp = existing_list;
        while (temp->next)
        {
            temp = temp->next;
        }
        temp->next = new_node;
        return existing_list;
    }
}

ASTNode *create_print_statement_node(ASTNode *expr)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_PRINT_STATEMENT;
    node->data.op.left = expr;
    return node;
}

ASTNode *create_error_statement_node(ASTNode *expr)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_ERROR_STATEMENT;
    node->data.op.left = expr;
    return node;
}

ASTNode *create_statement_list(ASTNode *statement, ASTNode *existing_list)
{
    if (!existing_list)
    {
        // If there's no existing list, create a new one
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = NODE_STATEMENT_LIST;
        node->data.statements = malloc(sizeof(StatementList));
        node->data.statements->statement = statement;
        node->data.statements->next = NULL;
        return node;
    }
    else
    {
        // Append at the end of existing_list
        StatementList *sl = existing_list->data.statements;
        while (sl->next)
        {
            sl = sl->next;
        }
        // Now sl is the last element; append the new statement
        StatementList *new_item = malloc(sizeof(StatementList));
        new_item->statement = statement;
        new_item->next = NULL;
        sl->next = new_item;
        return existing_list;
    }
}

int evaluate_expression(ASTNode *node)
{
    if (!node)
        return 0;
    switch (node->type)
    {
    case NODE_NUMBER:
        return node->data.value;
    case NODE_CHAR:
        return node->data.value;
    case NODE_IDENTIFIER:
        return get_variable(node->data.name);
    case NODE_ASSIGNMENT:
    {
        int value = evaluate_expression(node->data.op.right);
        set_variable(node->data.op.left->data.name, value);
        return value;
    }
    case NODE_OPERATION:
    {
        int left = evaluate_expression(node->data.op.left);
        int right = evaluate_expression(node->data.op.right);
        switch (node->data.op.op)
        {
        case OP_PLUS:
            return left + right;
        case OP_MINUS:
            return left - right;
        case OP_TIMES:
            return left * right;
        case OP_DIVIDE:
            return left / right;
        case OP_MOD:
            return left % right;
        case OP_LT:
            return left < right;
        case OP_GT:
            return left > right;
        case OP_LE:
            return left <= right;
        case OP_GE:
            return left >= right;
        case OP_EQ:
            return left == right;
        case OP_NE:
            return left != right;
        case OP_AND:
            return left && right;
        case OP_OR:
            return left || right;
        default:
            yyerror("Unknown operator");
            return 0;
        }
    }
    case NODE_UNARY_OPERATION:
    {
        int operand = evaluate_expression(node->data.unary.operand);
        switch (node->data.unary.op)
        {
        case OP_NEG:
            return -operand;
        default:
            yyerror("Unknown unary operator");
            return 0;
        }
    }
    case NODE_STRING_LITERAL:
        yyerror("Cannot evaluate a string literal as an integer");
        return 0;
    case NODE_FUNC_CALL:
    {
        // e.g. get function name
        char *name = node->data.func_call.function_name;
        ArgumentList *args = node->data.func_call.arguments;

        // If your language has only built-in functions like yapping, baka, etc.
        // you can do a simple if/else or switch:
        if (strcmp(name, "yapping") == 0)
        {
            execute_yapping_call(args);
            return 0;
        }
        if (strcmp(name, "yappin") == 0)
        {
            execute_yappin_call(args);
            return 0;
        }
        else if (strcmp(name, "baka") == 0)
        {
            execute_baka_call(args);
            return 0;
        }
        else
        {
            // If you want user-defined functions, you'd handle them here
            yyerror("Unknown function call");
            return 0;
        }
    }
    default:
        yyerror("Unknown expression type");
        return 0;
    }
}

void execute_statement(ASTNode *node)
{
    if (!node)
        return;
    switch (node->type)
    {
    case NODE_ASSIGNMENT:
    case NODE_OPERATION:
    case NODE_UNARY_OPERATION:
    case NODE_NUMBER:
    case NODE_CHAR:
    case NODE_IDENTIFIER:
        evaluate_expression(node);
        break;
    case NODE_FUNC_CALL:
        evaluate_expression(node);
        break;
    case NODE_FOR_STATEMENT:
        execute_for_statement(node);
        break;
    case NODE_WHILE_STATEMENT:
        execute_while_statement(node);
        break;
    case NODE_PRINT_STATEMENT:
    {
        ASTNode *expr = node->data.op.left;
        if (expr->type == NODE_STRING_LITERAL)
        {
            yapping("%s\n", expr->data.name);
        }
        else
        {
            int value = evaluate_expression(expr);
            yapping("%d\n", value);
        }
        break;
    }
    case NODE_ERROR_STATEMENT:
    {
        ASTNode *expr = node->data.op.left;
        if (expr->type == NODE_STRING_LITERAL)
        {
            baka("%s\n", expr->data.name);
        }
        else
        {
            int value = evaluate_expression(expr);
            baka("%d\n", value);
        }
        break;
    }
    case NODE_STATEMENT_LIST:
        execute_statements(node);
        break;
    case NODE_IF_STATEMENT:
        if (evaluate_expression(node->data.if_stmt.condition))
        {
            execute_statement(node->data.if_stmt.then_branch);
        }
        else if (node->data.if_stmt.else_branch)
        {
            execute_statement(node->data.if_stmt.else_branch);
        }
        break;
    case NODE_SWITCH_STATEMENT:
        execute_switch_statement(node);
        break;
    case NODE_BREAK_STATEMENT:
        // Signal to break out of the current loop/switch
        longjmp(break_env, 1);
        break;
    default:
        yyerror("Unknown statement type");
        break;
    }
}

void execute_statements(ASTNode *node)
{
    if (!node)
        return;
    if (node->type != NODE_STATEMENT_LIST)
    {
        execute_statement(node);
        return;
    }
    StatementList *current = node->data.statements;
    while (current)
    {
        execute_statement(current->statement);
        current = current->next;
    }
}

void execute_for_statement(ASTNode *node)
{
    evaluate_expression(node->data.for_stmt.init);
    while (evaluate_expression(node->data.for_stmt.cond))
    {
        execute_statement(node->data.for_stmt.body);
        evaluate_expression(node->data.for_stmt.incr);
    }
}

void execute_while_statement(ASTNode *node)
{
    while (evaluate_expression(node->data.while_stmt.cond))
    {
        execute_statement(node->data.while_stmt.body);
    }
}

ASTNode *create_if_statement_node(ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_IF_STATEMENT;
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.then_branch = then_branch;
    node->data.if_stmt.else_branch = else_branch;
    return node;
}

ASTNode *create_string_literal_node(char *string)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_STRING_LITERAL;
    node->data.name = string;
    return node;
}

ASTNode *create_switch_statement_node(ASTNode *expression, CaseNode *cases)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_SWITCH_STATEMENT;
    node->data.switch_stmt.expression = expression;
    node->data.switch_stmt.cases = cases;
    return node;
}

CaseNode *create_case_node(ASTNode *value, ASTNode *statements)
{
    CaseNode *node = malloc(sizeof(CaseNode));
    node->value = value;
    node->statements = statements;
    node->next = NULL;
    return node;
}

CaseNode *create_default_case_node(ASTNode *statements)
{
    return create_case_node(NULL, statements); // NULL value indicates default case
}

CaseNode *append_case_list(CaseNode *list, CaseNode *case_node)
{
    if (!list)
        return case_node;
    CaseNode *current = list;
    while (current->next)
        current = current->next;
    current->next = case_node;
    return list;
}

ASTNode *create_break_node()
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_BREAK_STATEMENT;
    node->data.break_stmt = NULL;
    return node;
}

void execute_yapping_call(ArgumentList *args)
{
    if (!args)
    {
        yapping("\n");
        return;
    }

    // Evaluate the first argument as a string (the format)
    // e.g. "Hello %d\n"
    ASTNode *formatNode = args->expr;
    if (formatNode->type != NODE_STRING_LITERAL)
    {
        yyerror("First argument to yapping must be a string literal");
        return;
    }
    const char *formatString = formatNode->data.name;

    // Move to next argument(s), which are the actual values
    ArgumentList *cur = args->next;

    // If you only want to handle exactly one additional integer:
    if (cur)
    {
        // Evaluate expression (e.g. 42)
        int val = evaluate_expression(cur->expr);

        // Pass that to yapping(...) as if you did yapping("Hello %d\n", 42);
        yapping(formatString, val);

        // You could handle more arguments if needed:
        // cur = cur->next;
        // ...
    }
    else
    {
        // No additional values, so just print the string
        yapping("%s", formatString);
    }
}

void execute_yappin_call(ArgumentList *args)
{
    if (!args)
    {
        yappin("\n");
        return;
    }

    // Evaluate the first argument as a string (the format)
    // e.g. "Hello %d\n"
    ASTNode *formatNode = args->expr;
    if (formatNode->type != NODE_STRING_LITERAL)
    {
        yyerror("First argument to yapping must be a string literal");
        return;
    }
    const char *formatString = formatNode->data.name;

    // Move to next argument(s), which are the actual values
    ArgumentList *cur = args->next;

    // If you only want to handle exactly one additional integer:
    if (cur)
    {
        // Evaluate expression (e.g. 42)
        int val = evaluate_expression(cur->expr);

        // Pass that to yapping(...) as if you did yapping("Hello %d\n", 42);
        yappin(formatString, val);

        // You could handle more arguments if needed:
        // cur = cur->next;
        // ...
    }
    else
    {
        // No additional values, so just print the string
        yappin("%s", formatString);
    }
}

void execute_baka_call(ArgumentList *args)
{
    if (!args)
    {
        baka("\n");
        return;
    }
    // parse the first argument as a format string
    // parse subsequent arguments as integers, etc.
    // call "baka(formatString, val, ...)"
}
