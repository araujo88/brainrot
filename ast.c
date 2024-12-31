/* ast.c */

#include "ast.h"
#include <stdbool.h>
#include <setjmp.h>
#include <string.h>

static jmp_buf break_env;

TypeModifiers current_modifiers = {false, false, false};

variable symbol_table[MAX_VARS];
int var_count = 0;

// Symbol table functions
bool set_int_variable(char *name, int value, TypeModifiers mods)
{
    for (int i = 0; i < var_count; i++)
    {
        if (strcmp(symbol_table[i].name, name) == 0)
        {
            symbol_table[i].is_float = false;
            symbol_table[i].value.ivalue = value;
            symbol_table[i].modifiers = mods;
            return true;
        }
    }

    if (var_count < MAX_VARS)
    {
        symbol_table[var_count].name = strdup(name);
        symbol_table[var_count].is_float = false;
        symbol_table[var_count].value.ivalue = value;
        symbol_table[var_count].modifiers = mods;
        var_count++;
        return true;
    }
    return false;
}

bool set_float_variable(char *name, float value, TypeModifiers mods)
{
    for (int i = 0; i < var_count; i++)
    {
        if (strcmp(symbol_table[i].name, name) == 0)
        {
            symbol_table[i].is_float = true;
            symbol_table[i].value.fvalue = value;
            symbol_table[i].modifiers = mods;
            return true;
        }
    }

    if (var_count < MAX_VARS)
    {
        symbol_table[var_count].name = strdup(name);
        symbol_table[var_count].is_float = true;
        symbol_table[var_count].value.fvalue = value;
        symbol_table[var_count].modifiers = mods;
        var_count++;
        return true;
    }
    return false;
}

void reset_modifiers(void)
{
    current_modifiers.is_volatile = false;
    current_modifiers.is_signed = false;
    current_modifiers.is_unsigned = false;
}

TypeModifiers get_current_modifiers(void)
{
    TypeModifiers mods = current_modifiers;
    reset_modifiers(); // Reset for next declaration
    return mods;
}

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
extern bool set_variable(char *name, int value, TypeModifiers mod);
extern int get_variable(char *name);
extern void yyerror(const char *s);
extern void yapping(const char *format, ...);
extern void yappin(const char *format, ...);
extern void baka(const char *format, ...);
extern TypeModifiers get_variable_modifiers(const char *name);

/* Function implementations */

ASTNode *create_number_node(int value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_NUMBER;
    node->data.value = value;
    return node;
}

ASTNode *create_float_node(float value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_FLOAT;
    node->data.fvalue = value;
    return node;
}

float evaluate_expression_float(ASTNode *node)
{
    if (!node)
        return 0.0f;

    switch (node->type)
    {
    case NODE_FLOAT:
        return node->data.fvalue;
    case NODE_NUMBER:
        return (float)node->data.value;
    case NODE_IDENTIFIER:
    {
        char *name = node->data.name;
        for (int i = 0; i < var_count; i++)
        {
            if (strcmp(symbol_table[i].name, name) == 0)
            {
                return symbol_table[i].is_float ? symbol_table[i].value.fvalue : (float)symbol_table[i].value.ivalue;
            }
        }
        yyerror("Undefined variable");
        return 0.0f;
    }
    case NODE_OPERATION:
    {
        float left = evaluate_expression_float(node->data.op.left);
        float right = evaluate_expression_float(node->data.op.right);

        switch (node->data.op.op)
        {
        case OP_PLUS:
            return left + right;
        case OP_MINUS:
            return left - right;
        case OP_TIMES:
            return left * right;
        case OP_DIVIDE:
            if (right == 0.0f)
            {
                yyerror("Division by zero");
                return 0.0f;
            }
            return left / right;
        case OP_LT:
            return left < right ? 1.0f : 0.0f;
        case OP_GT:
            return left > right ? 1.0f : 0.0f;
        case OP_LE:
            return left <= right ? 1.0f : 0.0f;
        case OP_GE:
            return left >= right ? 1.0f : 0.0f;
        case OP_EQ:
            return left == right ? 1.0f : 0.0f;
        case OP_NE:
            return left != right ? 1.0f : 0.0f;
        default:
            yyerror("Invalid operator for float operation");
            return 0.0f;
        }
    }
    case NODE_UNARY_OPERATION:
    {
        float operand = evaluate_expression_float(node->data.unary.operand);
        switch (node->data.unary.op)
        {
        case OP_NEG:
            return -operand;
        default:
            yyerror("Unknown unary operator for float");
            return 0.0f;
        }
    }
    default:
        yyerror("Invalid float expression");
        return 0.0f;
    }
}

int evaluate_expression_int(ASTNode *node)
{
    if (!node)
        return 0;

    switch (node->type)
    {
    case NODE_NUMBER:
        return node->data.value;
    case NODE_BOOLEAN:
        return node->data.value; // Already 1 or 0
    case NODE_FLOAT:
        yyerror("Cannot use float in integer context");
        return (int)node->data.fvalue;
    case NODE_IDENTIFIER:
    {
        char *name = node->data.name;
        for (int i = 0; i < var_count; i++)
        {
            if (strcmp(symbol_table[i].name, name) == 0)
            {
                if (symbol_table[i].is_float)
                {
                    yyerror("Cannot use float variable in integer context");
                    return (int)symbol_table[i].value.fvalue;
                }
                return symbol_table[i].value.ivalue;
            }
        }
        yyerror("Undefined variable");
        return 0;
    }
    case NODE_OPERATION:
    {
        // Special handling for logical operations
        if (node->data.op.op == OP_AND || node->data.op.op == OP_OR)
        {
            int left = evaluate_expression_int(node->data.op.left);
            int right = evaluate_expression_int(node->data.op.right);

            switch (node->data.op.op)
            {
            case OP_AND:
                return left && right;
            case OP_OR:
                return left || right;
            default:
                break;
            }
        }

        // Regular integer operations
        int left = evaluate_expression_int(node->data.op.left);
        int right = evaluate_expression_int(node->data.op.right);

        switch (node->data.op.op)
        {
        case OP_PLUS:
            return left + right;
        case OP_MINUS:
            return left - right;
        case OP_TIMES:
            return left * right;
        case OP_DIVIDE:
            if (right == 0)
            {
                yyerror("Division by zero");
                return 0;
            }
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
        default:
            yyerror("Unknown operator");
            return 0;
        }
    }
    case NODE_UNARY_OPERATION:
    {
        int operand = evaluate_expression_int(node->data.unary.operand);
        switch (node->data.unary.op)
        {
        case OP_NEG:
            return -operand;
        default:
            yyerror("Unknown unary operator");
            return 0;
        }
    }
    default:
        yyerror("Invalid integer expression");
        return 0;
    }
}

ASTNode *create_char_node(char value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_CHAR;
    node->data.value = value;
    return node;
}

ASTNode *create_boolean_node(int value)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_BOOLEAN;
    node->data.value = value ? 1 : 0;
    node->modifiers.is_boolean = true;
    return node;
}

ASTNode *create_sizeof_node(char *identifier)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_SIZEOF;
    node->data.name = strdup(identifier);
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
    node->modifiers = get_current_modifiers();
    if (expr->type == NODE_BOOLEAN)
    {
        node->modifiers.is_boolean = true;
    }
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

    node->modifiers.is_unsigned = left->modifiers.is_unsigned || right->modifiers.is_unsigned;
    node->modifiers.is_signed = false;
    node->modifiers.is_volatile = left->modifiers.is_volatile || right->modifiers.is_volatile;

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

bool is_float_expression(ASTNode *node)
{
    if (!node)
        return false;

    switch (node->type)
    {
    case NODE_FLOAT:
        return true;
    case NODE_NUMBER:
        return false;
    case NODE_IDENTIFIER:
    {
        for (int i = 0; i < var_count; i++)
        {
            if (strcmp(symbol_table[i].name, node->data.name) == 0)
            {
                return symbol_table[i].is_float;
            }
        }
        yyerror("Undefined variable in type check");
        return false;
    }
    case NODE_OPERATION:
    {
        // If either operand is float, result is float
        return is_float_expression(node->data.op.left) ||
               is_float_expression(node->data.op.right);
    }
    default:
        return false;
    }
}

int evaluate_expression(ASTNode *node)
{
    if (is_float_expression(node))
    {
        return (int)evaluate_expression_float(node);
    }
    return evaluate_expression_int(node);
}

void execute_assignment(ASTNode *node)
{
    if (node->type != NODE_ASSIGNMENT)
    {
        yyerror("Expected assignment node");
        return;
    }

    char *name = node->data.op.left->data.name;
    ASTNode *value_node = node->data.op.right;
    TypeModifiers mods = node->modifiers;

    // Check if the right-hand side is a float expression
    if (is_float_expression(value_node))
    {
        float value = evaluate_expression_float(value_node);
        if (!set_float_variable(name, value, mods))
        {
            yyerror("Failed to set float variable");
        }
    }
    else
    {
        int value = evaluate_expression_int(value_node);
        if (!set_int_variable(name, value, mods))
        {
            yyerror("Failed to set integer variable");
        }
    }
}

void execute_statement(ASTNode *node)
{
    if (!node)
        return;
    switch (node->type)
    {
    case NODE_ASSIGNMENT:
        execute_assignment(node);
        break;
    case NODE_OPERATION:
    case NODE_UNARY_OPERATION:
    case NODE_NUMBER:
    case NODE_CHAR:
    case NODE_IDENTIFIER:
        evaluate_expression(node);
        break;
    case NODE_FUNC_CALL:
        if (strcmp(node->data.func_call.function_name, "yapping") == 0)
        {
            execute_yapping_call(node->data.func_call.arguments);
        }
        else if (strcmp(node->data.func_call.function_name, "yappin") == 0)
        {
            execute_yappin_call(node->data.func_call.arguments);
        }
        else if (strcmp(node->data.func_call.function_name, "baka") == 0)
        {
            execute_baka_call(node->data.func_call.arguments);
        }
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

    ASTNode *formatNode = args->expr;
    if (formatNode->type != NODE_STRING_LITERAL)
    {
        yyerror("First argument to yapping must be a string literal");
        return;
    }

    ArgumentList *cur = args->next;
    if (!cur)
    {
        yapping("%s", formatNode->data.name);
        return;
    }

    ASTNode *expr = cur->expr;

    // Special handling for sizeof
    if (expr->type == NODE_SIZEOF ||
        (expr->type == NODE_IDENTIFIER && strstr(formatNode->data.name, "%lu") != NULL))
    {
        int val = evaluate_expression_int(expr);
        yapping(formatNode->data.name, (unsigned long)val);
        return;
    }

    // Handle float expressions first
    if (is_float_expression(expr))
    {
        float val = evaluate_expression_float(expr);
        yapping(formatNode->data.name, val);
        return;
    }

    // Handle boolean values
    bool is_bool = false;
    if (expr->type == NODE_BOOLEAN)
    {
        is_bool = true;
    }
    else if (expr->type == NODE_IDENTIFIER)
    {
        TypeModifiers mods = get_variable_modifiers(expr->data.name);
        is_bool = mods.is_boolean;
    }

    if (is_bool)
    {
        int val = evaluate_expression_int(expr);
        yapping(val ? "yes" : "no");
        return;
    }

    // Handle integers
    int val = evaluate_expression_int(expr);
    yapping(formatNode->data.name, val);
}

void execute_yappin_call(ArgumentList *args)
{
    if (!args)
    {
        yappin("\n");
        return;
    }

    ASTNode *formatNode = args->expr;
    if (formatNode->type != NODE_STRING_LITERAL)
    {
        yyerror("First argument to yappin must be a string literal");
        return;
    }

    ArgumentList *cur = args->next;
    if (!cur)
    {
        yappin("%s", formatNode->data.name);
        return;
    }

    ASTNode *expr = cur->expr;

    // Check if it's a boolean value
    if (expr->type == NODE_BOOLEAN ||
        (expr->type == NODE_IDENTIFIER && get_variable_modifiers(expr->data.name).is_boolean))
    {
        int val = evaluate_expression_int(expr);
        if (strstr(formatNode->data.name, "%d") != NULL)
        {
            yappin(formatNode->data.name, val);
        }
        else
        {
            yappin(val ? "yes" : "no");
        }
        return;
    }

    // Handle float expressions
    if (is_float_expression(expr))
    {
        float val = evaluate_expression_float(expr);
        yappin(formatNode->data.name, val);
        return;
    }

    // Handle integer expressions
    int val = evaluate_expression_int(expr);
    yappin(formatNode->data.name, val);
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
