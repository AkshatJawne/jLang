#include <ctype.h>
#include <stdio.h>
#define DS_IMPLEMENTATION
#include "ds.h"
#include <string.h>

// ##################################################################
// #                                                                #
// #  jLang  & Compiler                                             #
// #                                                                #
// ##################################################################

// Token types for the compiler
enum token_type
{
    IDENT,
    LABEL,
    INT,
    INPUT,
    OUTPUT,
    GOTO,
    IF,
    THEN,
    EQUAL,
    PLUS,
    LESS_THAN,
    INVALID,
    END
};

// Represents a token in the input stream
struct token
{
    enum token_type type;
    char *value;
};

// Function that returns token type
const char *show_token_type(enum token_type type)
{
    switch (type)
    {
    case IDENT:
        return "ident";
    case LABEL:
        return "label";
    case INT:
        return "int";
    case INPUT:
        return "input";
    case OUTPUT:
        return "output";
    case GOTO:
        return "goto";
    case IF:
        return "if";
    case THEN:
        return "then";
    case EQUAL:
        return "equal";
    case PLUS:
        return "plus";
    case LESS_THAN:
        return "less_than";
    case INVALID:
        return "invalid";
    case END:
        return "end";
    }
}

// Function that prints the type and value of a token
void print_token(struct token tok)
{
    const char *type = show_token_type(tok.type);
    printf("%s", type);
    if (tok.value != NULL)
    {
        printf("(%s)", tok.value);
    }
    printf("\n");
}

// Lexer breaks input character stream into tokens
struct lexer
{
    char *buffer;
    unsigned int buffer_len;
    unsigned int pos;
    unsigned int read_pos;
    char ch;
};

// Function that looks at the char in the read_pos in the input buffer without returning it
static char lexer_peek_char(struct lexer *l)
{
    if (l->read_pos >= l->buffer_len)
    {
        // if the read position is greater than length buffer, we are at EOF (end of file)
        return EOF;
    }

    // return char at read_pos index
    return l->buffer[l->read_pos];
}

// Function that returns the char at the read_pos, and advances the read_pos to the next char
static char lexer_read_char(struct lexer *l)
{
    // calls on lexer_peek_char to get return value and store in ch position
    l->ch = lexer_peek_char(l);

    // advance new pos to be read_pos
    l->pos = l->read_pos;
    l->read_pos += 1;

    return l->ch;
}

// Function that positions lexer at beginning of next token by skipping over any leading whitespace
static void skip_whitespaces(struct lexer *l)
{
    // if whitespace, call lexer_read_char func, else continue
    while (isspace(l->ch))
    {
        lexer_read_char(l);
    }
}

// Function that initializes a new lexer instance
static void lexer_init(struct lexer *l, char *buffer, unsigned int buffer_len)
{
    l->buffer = buffer;
    l->buffer_len = buffer_len;
    l->pos = 0;
    l->read_pos = 0;
    l->ch = 0;

    // read first character
    lexer_read_char(l);
}

// Function that handles core lexer logic
static struct token lexer_next_token(struct lexer *l)
{
    // advance to first non-whitespace character
    skip_whitespaces(l);

    // Return END token
    if (l->ch == EOF)
    {
        lexer_read_char(l);
        return (struct token){.type = END, .value = NULL};
    }
    // Return EQUAL token
    else if (l->ch == '=')
    {
        lexer_read_char(l);
        return (struct token){.type = EQUAL, .value = NULL};
    }
    // Return LESS_THAN token
    else if (l->ch == '<')
    {
        lexer_read_char(l);
        return (struct token){.type = LESS_THAN, .value = NULL};
    }
    // Return PLUS token
    else if (l->ch == '+')
    {
        lexer_read_char(l);
        return (struct token){.type = PLUS, .value = NULL};
    }
    // : indicates we are dealing with a series of alphanumeric characters (a label)
    else if (l->ch == ':')
    {
        lexer_read_char(l);
        // create slice struct that initializes str to point to current_pos, with 0 length
        ds_string_slice slice = {.str = l->buffer + l->pos, .len = 0};

        // Increment length while we have alphanumeric char or underscore
        while (isalnum(l->ch) || l->ch == '_')
        {
            slice.len += 1;
            lexer_read_char(l);
        }
        char *value = NULL;
        // create new owned string using slice and store in value pointer
        ds_string_slice_to_owned(&slice, &value);

        // create token with label and previosuely determined value
        return (struct token){.type = LABEL, .value = value};
    }
    else if (isdigit(l->ch))
    {
        // similar logic to :, just return INT token
        ds_string_slice slice = {.str = l->buffer + l->pos, .len = 0};
        while (isdigit(l->ch))
        {
            slice.len += 1;
            lexer_read_char(l);
        }
        char *value = NULL;
        ds_string_slice_to_owned(&slice, &value);
        return (struct token){.type = INT, .value = value};
    }
    else if (isalnum(l->ch) || l->ch == '_')
    {
        // similar logic to conditional above
        ds_string_slice slice = {.str = l->buffer + l->pos, .len = 0};
        while (isalnum(l->ch) || l->ch == '_')
        {
            slice.len += 1;
            lexer_read_char(l);
        }
        char *value = NULL;
        ds_string_slice_to_owned(&slice, &value);

        // check if the identfied value is a keyword

        // Return input keyword token
        if (strcmp(value, "input") == 0)
        {
            return (struct token){.type = INPUT, .value = NULL};
        }
        // Return output keyword token
        else if (strcmp(value, "output") == 0)
        {
            return (struct token){.type = OUTPUT, .value = NULL};
        }
        // Return if keyword token
        else if (strcmp(value, "if") == 0)
        {
            return (struct token){.type = IF, .value = NULL};
        }
        // Return then keyword token
        else if (strcmp(value, "then") == 0)
        {
            return (struct token){.type = THEN, .value = NULL};
        }
        // Return goto keyword token
        else if (strcmp(value, "goto") == 0)
        {
            return (struct token){.type = GOTO, .value = NULL};
        }
        // Return identifier token
        else
        {
            return (struct token){.type = IDENT, .value = value};
        }
    }
    else
    {
        // return an invalid token for characters that do not fall into conditionals above
        ds_string_slice slice = {.str = l->buffer + l->pos, .len = 1};
        char *value = NULL;
        ds_string_slice_to_owned(&slice, &value);
        lexer_read_char(l);
        return (struct token){.type = INVALID, .value = value};
    }
}

// Function that tokenizes input buffer and stores tokens in an dynamic array data structure
int lexer_tokenize(char *buffer, unsigned int length,
                   ds_dynamic_array *tokens)
{
    struct lexer lexer;
    // Initialize the lexer with the input buffer and length parameters
    lexer_init(&lexer, (char *)buffer, length);

    struct token tok;
    do
    {
        // get next token
        tok = lexer_next_token(&lexer);
        if (ds_dynamic_array_append(tokens, &tok) != 0)
        {
            // throw error if we cannot append to end of dynamic array
            DS_PANIC("Failed to append token to array");
        }
    } while (tok.type != END);

    return 0;
}

// Defining structures for Abstract Syntax Tree (AST)

// Types of possible terms (input, integers, and identifiers)
enum term_type
{
    TERM_INPUT,
    TERM_INT,
    TERM_IDENT
};

// Represents a term in our JLang language
struct term_node
{
    enum term_type type;
    union
    {
        char *value;
    };
};

// Types of possible expressions (unary term, or binary expression)
enum expr_type
{
    EXPR_TERM,
    EXPR_PLUS,
};

// Represents a binary expression with two terms (LHS and RHS)
struct term_binary_node
{
    struct term_node lhs;
    struct term_node rhs;
};

// General struct for expressions, can be either binary or single term
struct expr_node
{
    enum expr_type type;
    union
    {
        struct term_node term;
        struct term_binary_node add;
    };
};

// The types of relation operations (less than, intending to add more in future)
enum rel_type
{
    REL_LESS_THAN,
};

// Outlines a node with our relational expression (less than)
struct rel_node
{
    enum rel_type type;
    union
    {
        struct term_binary_node less_than;
    };
};

// Specifies the possible instruction types (assignment, if conditionals, goto, outputs, and labels)
enum instr_type
{
    INSTR_ASSIGN,
    INSTR_IF,
    INSTR_GOTO,
    INSTR_OUTPUT,
    INSTR_LABEL
};

// Node for if we are dealing with an assignment instruction, housing identifier and expression
struct assign_node
{
    char *ident;
    struct expr_node expr;
};

// Node for if we are dealing with an if conditional, housing instruction node for if it is true
struct if_node
{
    struct rel_node rel;
    struct instr_node *instr;
};

// Node for if we are dealing with goto statement, housing label where directed instruction block begins
struct goto_node
{
    char *label;
};

// Node for if we are dealing with an output, housing term to be displayed
struct output_node
{
    struct term_node term;
};

// Node for if we are dealing with a label, housing value
struct label_node
{
    char *label;
};

// General node that holds type, and union to hold different instruction node types
struct instr_node
{
    enum instr_type type;
    union
    {
        struct assign_node assign;
        struct if_node if_;
        struct goto_node goto_;
        struct output_node output;
        struct label_node label;
    };
};

// Node to represent an array of instructions (given that we can have multi-line programs)
struct program_node
{
    ds_dynamic_array instrs;
};

// Struct to represent parser with array of tokens and corresponding array index
struct parser
{
    ds_dynamic_array tokens;
    unsigned int index;
};

// Function to intiialize parser instance
void parser_init(ds_dynamic_array tokens, struct parser *p)
{
    p->tokens = tokens;
    p->index = 0;
}

// Function to return current token from tokens array at index
void parser_current(struct parser *p, struct token *token)
{
    ds_dynamic_array_get(&p->tokens, p->index, token);
}

// advance the current index in the tokens array
void parser_advance(struct parser *p) { p->index++; }

// FUnction to parse a token from the given token stream
void parse_term(struct parser *p, struct term_node *term)
{
    struct token token;

    parser_current(p, &token);

    // determine type of token, and assign value based on corresponding node struct
    if (token.type == INPUT)
    {
        term->type = TERM_INPUT;
    }
    else if (token.type == INT)
    {
        term->type = TERM_INT;
        term->value = token.value;
    }
    else if (token.type == IDENT)
    {
        term->type = TERM_IDENT;
        term->value = token.value;
    }
    else
    {
        // throw error if the type is not expected
        DS_PANIC("Expected a term (input, int or ident) but found %s",
                 show_token_type(token.type));
    }

    // advance to next token in stream
    parser_advance(p);
}

// Function to transform single terms and binary expressions into expression nodes
void parse_expr(struct parser *p, struct expr_node *expr)
{
    struct token token;
    struct term_node lhs, rhs;

    parse_term(p, &lhs);

    // get current token from term
    parser_current(p, &token);

    // plus is the only binary expression operator
    if (token.type == PLUS)
    {
        parser_advance(p);
        parse_term(p, &rhs);

        // create binary expression node
        expr->type = EXPR_PLUS;
        expr->add.lhs = lhs;
        expr->add.rhs = rhs;
    }
    else
    {
        // create single expression node
        expr->type = EXPR_TERM;
        expr->term = lhs;
    }
}

// Function to turn relation operation expressions into corresponding nodes
void parse_rel(struct parser *p, struct rel_node *rel)
{
    struct token token;
    struct term_node lhs, rhs;

    parse_term(p, &lhs);

    // get current token from term
    parser_current(p, &token);

    // Conditional to check if < (assuming more relation operators are added in the future)
    if (token.type == LESS_THAN)
    {
        parser_advance(p);
        parse_term(p, &rhs);

        // create relation operation node
        rel->type = REL_LESS_THAN;
        rel->less_than.lhs = lhs;
        rel->less_than.rhs = rhs;
    }
    else
    {
        // Throw error if we get a relation operator that is not <
        DS_PANIC("Expected rel (<) found %s", show_token_type(token.type));
    }
}

// Function to turn assignment operation expressions into corresponding nodes
void parse_assign(struct parser *p, struct instr_node *instr)
{
    struct token token;

    // set instruction type to assignment
    instr->type = INSTR_ASSIGN;

    parser_current(p, &token);
    instr->assign.ident = token.value;

    // consume the identifier token
    parser_advance(p);
    parser_current(p, &token);
    if (token.type != EQUAL)
    {
        // throw error if we are given an assignment operator that is not =
        DS_PANIC("Expected equal found %s", show_token_type(token.type));
    }

    // consume the operator (=) token
    parser_advance(p);

    // parse the expression to get actual assignment
    parse_expr(p, &instr->assign.expr);
}

// Forward declaration for parse_instr, since we will use it in parse_if
void parse_instr(struct parser *p, struct instr_node *instr);

// Function to turn if operation expressions into corresponding nodes
void parse_if(struct parser *p, struct instr_node *instr)
{
    struct token token;

    // assign type to if
    instr->type = INSTR_IF;
    parser_advance(p);

    // parse the relation to get the conditional
    parse_rel(p, &instr->if_.rel);

    parser_current(p, &token);
    if (token.type != THEN)
    {
        // if we get unexpected syntax for statement after conditional
        DS_PANIC("Expected then found %s", show_token_type(token.type));
    }

    parser_advance(p);

    // Allocate memory and parse instruction for if
    instr->if_.instr = malloc(sizeof(struct instr_node));
    parse_instr(p, instr->if_.instr);
}

// Function to parse goto expressions
void parse_goto(struct parser *p, struct instr_node *instr)
{
    struct token token;

    // Assign instruction type to be goto
    instr->type = INSTR_GOTO;
    parser_advance(p);

    parser_current(p, &token);
    if (token.type != LABEL)
    {
        // if label cannot be processed
        DS_PANIC("Expected label found %s", show_token_type(token.type));
    }
    parser_advance(p);

    // set goto label to be value of goto token
    instr->goto_.label = token.value;
}

// Function that parses an output instruction
void parse_output(struct parser *p, struct instr_node *instr)
{
    struct token token;
    struct term_node lhs;

    // Assign instruction type to be output
    instr->type = INSTR_OUTPUT;
    parser_advance(p);

    // parse the term to be outputted
    parse_term(p, &lhs);

    // set the term in the output instruction
    instr->output.term = lhs;
}

// Function to parse labels in code and assign corresponding node values
void parse_label(struct parser *p, struct instr_node *instr)
{
    struct token token;

    // Assign instruction type to be label
    instr->type = INSTR_LABEL;

    parser_current(p, &token);
    // Set the value of the label name
    instr->label.label = token.value;

    // consume the label token
    parser_advance(p);
}

// Function to parse instructions, given instruction type
void parse_instr(struct parser *p, struct instr_node *instr)
{
    struct token token;

    // Get current token
    parser_current(p, &token);

    // Call different parser functions based on token type

    if (token.type == IDENT)
    {
        parse_assign(p, instr);
    }
    else if (token.type == IF)
    {
        parse_if(p, instr);
    }
    else if (token.type == GOTO)
    {
        parse_goto(p, instr);
    }
    else if (token.type == OUTPUT)
    {
        parse_output(p, instr);
    }
    else if (token.type == LABEL)
    {
        parse_label(p, instr);
    }
    else
    {
        // throw if we have a bad token type
        DS_PANIC("unexpected token %s", show_token_type(token.type));
    }
}

// Function to return value in dynamic array of tokens
int find_variable(ds_dynamic_array *variables, char *ident)
{
    for (unsigned int i = 0; i < variables->count; i++)
    {
        char *variable = NULL;
        ds_dynamic_array_get(variables, i, &variable);

        // use strcmp to compare two strings
        if (strcmp(ident, variable) == 0)
        {
            return i;
        }
    }

    return -1;
}

// Function that reads program and builds AST
void parse_program(struct parser *p, struct program_node *program)
{
    ds_dynamic_array_init(&program->instrs, sizeof(struct instr_node));

    struct token token;
    do
    {
        struct instr_node instr;

        parse_instr(p, &instr);

        // add parsed unsrtuction to the end of instruction array
        ds_dynamic_array_append(&program->instrs, &instr);

        // get next token
        parser_current(p, &token);
    } while (token.type != END);
}

// Function that generates assembly for a given term
void term_asm(struct term_node *term, ds_dynamic_array *variables)
{
    switch (term->type)
    {

    // If we are dealing with input expression
    case TERM_INPUT:
    {
        // read a number and store it in rax x86 register
        printf("    read 0, line, LINE_MAX\n");
        printf("    mov rdi, line\n");
        printf("    call strlen\n");
        printf("    mov rdi, line\n");
        printf("    mov rsi, rax\n");
        printf("    call parse_uint\n");
        break;
    }

    // If we are dealing with identifier term
    case TERM_IDENT:
    {
        int index = find_variable(variables, term->value);
        printf("    mov rax, qword [rbp - %d]\n", index * 8 + 8);
        break;
    }

    // If we are dealing with integer term
    case TERM_INT:
        printf("    mov rax, %s\n", term->value);
        break;
    }
}

// Function that generates assembly for given expression
void expr_asm(struct expr_node *expr, ds_dynamic_array *variables)
{
    switch (expr->type)
    {

    // If we are dealing with unary expression
    case EXPR_TERM:
    {
        // generate assembly for single term using previousely declared function
        term_asm(&expr->term, variables);
        break;
    }

    // If we are dealing with a binary expression
    case EXPR_PLUS:
        term_asm(&expr->add.lhs, variables);
        printf("    mov rdx, rax\n");
        term_asm(&expr->add.rhs, variables);
        // add result of generated assembly of rhs stored in rax to left hand result stores in rdx
        printf("    add rax, rdx\n");
        break;
    }
}

// Function to generate assembly for relation operators (<)
void rel_asm(struct rel_node *rel, ds_dynamic_array *variables)
{
    switch (rel->type)
    {
    case REL_LESS_THAN:
        term_asm(&rel->less_than.lhs, variables);
        printf("    mov rdx, rax\n");
        term_asm(&rel->less_than.rhs, variables);
        // Compare the left-hand side (rdx) with the right-hand side (rax)
        printf("    cmp rdx, rax\n");
        // Set the least significant bit of al to 1 if rdx is less than rax
        printf("    setl al\n");
        // Clear remaning bits and zero ex-tend al to rax (to ensure consistent data size)
        printf("    and al, 1\n");
        printf("    movzx rax, al\n");
        break;
    }
}

// Function to generate assembly code for a given instruction node
void instr_asm(struct instr_node *instr, ds_dynamic_array *variables,
               int *if_count)
{
    switch (instr->type)
    {
    // if dealing with assignment node
    case INSTR_ASSIGN:
    {
        expr_asm(&instr->assign.expr, variables); // the result is in rax
        int index = find_variable(variables, instr->assign.ident);
        printf("    mov qword [rbp - %d], rax\n", index * 8 + 8);
        break;
    }
    // if dealing with if statement
    case INSTR_IF:
    {
        rel_asm(&instr->if_.rel, variables); // the result is in rax
        int label = (*if_count)++;
        printf("    test rax, rax\n");
        printf("    jz .endif%d\n", label);
        instr_asm(instr->if_.instr, variables, if_count);
        printf(".endif%d:\n", label);
        break;
    }
    // if dealing with goto statement
    case INSTR_GOTO:
    {
        printf("    jmp .%s\n", instr->goto_.label);
        break;
    }
    // if dealing with output
    case INSTR_OUTPUT:
    {
        term_asm(&instr->output.term, variables);
        printf("    mov rdi, 1\n");
        printf("    mov rsi, rax\n");
        printf("    call write_uint\n");
        break;
    }
    // if dealing with a label
    case INSTR_LABEL:
        printf(".%s:\n", instr->label.label);
        break;
    }
}

// Function that checks if we have a variable declaration
void term_declare_variables(struct term_node *term,
                            ds_dynamic_array *variables)
{
    switch (term->type)
    {
    // For first two cases, we have no variables to declare
    case TERM_INPUT:
        break;
    case TERM_INT:
        break;
    case TERM_IDENT:
        for (unsigned int i = 0; i < variables->count; i++)
        {
            char *variable = NULL;
            ds_dynamic_array_get(variables, i, &variable);

            // Here we check of the variable is already declared
            if (strcmp(term->value, variable) == 0)
            {
                return;
            }
        }
        // Throw an error if the variable is not defined
        DS_PANIC("Identifier is not defined %s", term->value);
        break;
    }
}

// Function that declares variables use in a expression (unary or binary)
void expr_declare_variables(struct expr_node *expr,
                            ds_dynamic_array *variables)
{
    switch (expr->type)
    {
    case EXPR_TERM:
    {
        // Recursively declare single term
        term_declare_variables(&expr->term, variables);
        break;
    }
    case EXPR_PLUS:
        // Recursively declare variables for left and right side of binary expression
        term_declare_variables(&expr->add.lhs, variables);
        term_declare_variables(&expr->add.rhs, variables);
        break;
    }
}

// Function that that declares variables used in a relational expression
void rel_declare_variables(struct rel_node *rel, ds_dynamic_array *variables)
{
    switch (rel->type)
    {
    case REL_LESS_THAN:
        // Recursively declare variables for left and right side
        term_declare_variables(&rel->less_than.lhs, variables);
        term_declare_variables(&rel->less_than.rhs, variables);
        break;
    }
}

// Function responsible for declaring variables used within an instruction
void instr_declare_variables(struct instr_node *instr,
                             ds_dynamic_array *variables)
{
    switch (instr->type)
    {
    // if dealing with an instruction assignment
    case INSTR_ASSIGN:
    {
        expr_declare_variables(&instr->assign.expr, variables);
        for (unsigned int i = 0; i < variables->count; i++)
        {
            char *variable = NULL;
            ds_dynamic_array_get(variables, i, &variable);

            // Here we check of the variable is already declared
            if (strcmp(instr->assign.ident, variable) == 0)
            {
                return;
            }
        }
        // append variables to the end of the dynamic token array
        ds_dynamic_array_append(variables, &instr->assign.ident);
        break;
    }
    // if dealing with an if statement
    case INSTR_IF:
    {
        rel_declare_variables(&instr->if_.rel, variables);
        instr_declare_variables(instr->if_.instr, variables);
        break;
    }
    // We can just break here, since no variables are declared in goto
    case INSTR_GOTO:
    {
        break;
    }
    // For when we are dealing with output
    case INSTR_OUTPUT:
    {
        term_declare_variables(&instr->output.term, variables);
        break;
    }
    // We can just break here, since no variables are declared in a label node
    case INSTR_LABEL:
        break;
    }
}

// Function responsible for generating the final assembly code through AST
void program_asm(struct program_node *program)
{
    int if_count = 0;
    ds_dynamic_array variables;
    ds_dynamic_array_init(&variables, sizeof(char *));

    // Iterate through and declare variables
    for (unsigned int i = 0; i < program->instrs.count; i++)
    {
        struct instr_node instr;
        ds_dynamic_array_get(&program->instrs, i, &instr);

        instr_declare_variables(&instr, &variables);
    }

    // Header for Assembly Executable Format (ELF64)
    printf("format ELF64 executable\n");
    printf("LINE_MAX equ 1024\n");
    printf("segment readable executable\n");
    printf("include \"linux.inc\"\n");
    printf("include \"utils.inc\"\n");
    printf("entry _start\n");
    printf("_start:\n");

    // Set up stack and allocate space for all declared variables (assuming 64 bit variables)
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, %d\n", variables.count * 8);

    for (unsigned int i = 0; i < program->instrs.count; i++)
    {
        // Iterate through and generate assembly code for each instruction
        struct instr_node instr;
        ds_dynamic_array_get(&program->instrs, i, &instr);

        instr_asm(&instr, &variables, &if_count);
    }

    // Stack Cleanup and Program Termination
    printf("    add rsp, %d\n", variables.count * 8);

    printf("    mov rax, 60\n");
    printf("    xor rdi, rdi\n");
    printf("    syscall\n");

    printf("segment readable writeable\n");
    printf("line rb LINE_MAX\n");
}

int main()
{
    char *buffer = NULL;
    // read input file
    int length = ds_io_read_file(NULL, &buffer);

    // initialize token array
    ds_dynamic_array tokens;
    ds_dynamic_array_init(&tokens, sizeof(struct token));

    // tokenize the input buffer
    lexer_tokenize(buffer, length, &tokens);

    struct parser p;
    struct program_node program;

    // Initialize parser and parse program into Abstract Syntaz Tree
    parser_init(tokens, &p);
    parse_program(&p, &program);

    // Generate Assembly for Program
    program_asm(&program);
}