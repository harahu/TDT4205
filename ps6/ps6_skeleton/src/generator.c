#include "vslc.h"
#include "genstack.h"

#define MIN(a,b) (((a)<(b)) ? (a):(b))
#define RECUR(nd, fn) do {				\
    for ( size_t i=0; i<(nd)->n_children; i++ ) \
        generate_node ( (nd)->children[i], fn );	\
} while ( false )

#define LBL(name) puts ( #name":" )
#define ASM0(op) puts ( "\t"#op )
#define ASM1(op,a) puts ( "\t"#op"\t"#a )
#define ASM2(op,a,b) puts ( "\t"#op"\t"#a", "#b )

static const char *record[6] = {
    "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
};

static void generate_node ( node_t *node, symbol_t *function );

size_t condcnt;
genstack_t *condstack;

size_t loopcnt;
genstack_t *loopstack;

static void
generate_stringtable ( void )
{
    puts ( ".section .rodata" );
    puts ( "intout: .string \"\%ld \"" );
    puts ( "strout: .string \"\%s \"" );
    puts ( "errout: .string \"Wrong number of arguments\"" );
    for ( size_t s=0; s<stringc; s++ )
        printf ( "STR%zu: .string %s\n", s, string_list[s] );
}

static void
generate_global_variables ( void )
{
    puts ( ".section .data" );
    size_t nsyms = tlhash_size ( global_names );
    symbol_t *syms[nsyms];
    tlhash_values ( global_names, (void **)&syms );
    for ( size_t n=0; n<nsyms; n++ )
    {
        if ( syms[n]->type == SYM_GLOBAL_VAR )
            printf ( "_%s: .zero 8\n", syms[n]->name );
    }
}

static void
generate_main ( symbol_t *first )
{
    puts ( ".globl main" );
    puts ( ".section .text" );
    puts ( "main:" );
    puts ( "\tpushq %rbp" );
    puts ( "\tmovq %rsp, %rbp" );

    puts ( "\tsubq $1, %rdi" );
    printf ( "\tcmpq\t$%zu,%%rdi\n", first->nparms );
    puts ( "\tjne ABORT" );
    puts ( "\tcmpq $0, %rdi" );
    puts ( "\tjz SKIP_ARGS" );

    puts ( "\tmovq %rdi, %rcx" );
    printf ( "\taddq $%zu, %%rsi\n", 8*first->nparms );
    puts ( "PARSE_ARGV:" );
    puts ( "\tpushq %rcx" );
    puts ( "\tpushq %rsi" );

    puts ( "\tmovq (%rsi), %rdi" );
    puts ( "\tmovq $0, %rsi" );
    puts ( "\tmovq $10, %rdx" );
    puts ( "\tcall strtol" );

    /*  Now a new argument is an integer in rax */
    puts ( "\tpopq %rsi" );
    puts ( "\tpopq %rcx" );
    puts ( "\tpushq %rax" );
    puts ( "\tsubq $8, %rsi" );
    puts ( "\tloop PARSE_ARGV" );

    /* Now the arguments are in order on stack */
    for ( int arg=0; arg<MIN(6,first->nparms); arg++ )
        printf ( "\tpopq\t%s\n", record[arg] );

    puts ( "SKIP_ARGS:" );
    printf ( "\tcall\t_%s\n", first->name );
    puts ( "\tjmp END" );
    puts ( "ABORT:" );
    puts ( "\tmovq $errout, %rdi" );
    puts ( "\tcall puts" );

    puts ( "END:" );
    puts ( "\tmovq %rax, %rdi" );
    puts ( "\tcall exit" );
}

static void
generate_identifier ( node_t *ident, symbol_t *function )
{
    symbol_t *symbol = ident->entry;
    switch ( symbol->type )
    {
        case SYM_GLOBAL_VAR:
            printf ( "_%s", symbol->name );
            break;
        case SYM_PARAMETER:
            if ( symbol->seq > 5 )
                    printf ( "%ld(%%rbp)", 8+8*(symbol->seq-5) );
            else
                printf ( "%ld(%%rbp)", -8*(symbol->seq+1) );
            break;
        case SYM_LOCAL_VAR:
	    if ( function->nparms >= 6 ) {
	        printf ( "%ld(%%rbp)", -8*(6+symbol->seq+1) );
	    }
	    else {
	      printf ( "%ld(%%rbp)", -8*(function->nparms+symbol->seq+1) );
	    }
	    break;
    }
}

static void
generate_expression ( node_t *expr, symbol_t *function )
{
    if ( expr->type == IDENTIFIER_DATA )
    {
        printf ( "\tmovq\t" );
        generate_identifier ( expr, function );
        printf ( ", %%rax\n" );
    }
    else if ( expr->type == NUMBER_DATA )
    {
        printf ( "\tmovq\t$%lld, %%rax\n", *(int64_t *)expr->data );
    }
    else if ( expr->n_children == 1 )
    {
        generate_expression ( expr->children[0], function );
        ASM1 ( negq, %rax );
    }
    else if ( expr->n_children == 2 )
    {
        if ( expr->data != NULL )
        {
            switch ( *((char *)expr->data) )
            {
                case '+':
		    generate_expression ( expr->children[0], function );
                    ASM1 ( pushq, %rax );
                    generate_expression ( expr->children[1], function );
                    ASM2 ( addq, %rax, (%rsp) );
                    ASM1 ( popq, %rax );
                    break;
                case '-':
		    generate_expression ( expr->children[0], function );
                    ASM1 ( pushq, %rax );
                    generate_expression ( expr->children[1], function );
                    ASM2 ( subq, %rax, (%rsp) );
                    ASM1 ( popq, %rax );
                    break;
                case '*':
                    ASM1 ( pushq, %rdx );
                    generate_expression ( expr->children[1], function );
                    ASM1 ( pushq, %rax );
                    generate_expression ( expr->children[0], function );
                    ASM0 ( cqo );
                    ASM1 ( imulq, (%rsp) );
                    ASM1 ( popq, %rdx );
                    ASM1 ( popq, %rdx );
                    break;
                case '/':
                    ASM1 ( pushq, %rdx );
                    generate_expression ( expr->children[1], function );
                    ASM1 ( pushq, %rax );
                    generate_expression ( expr->children[0], function );
                    ASM0 ( cqo );
                    ASM1 ( idivq, (%rsp) );
                    ASM1 ( popq, %rdx );
                    ASM1 ( popq, %rdx );
                    break;
            }
        }
	else {
	    if ( expr->children[0]->type == IDENTIFIER_DATA && expr->children[1]->type == EXPRESSION_LIST ) {
	        //Save registers
	        for ( int i = 0; i < 6; i++ ) {
	            printf ( "\tpushq\t%s\n", record[i] );
	        }
		//Load stack with arguments
		node_t **param_list = expr->children[1]->children;
		for ( int i = expr->children[1]->n_children-1; i >= 0; i-- ) {
		    generate_expression ( param_list[i], function );
		    ASM1 ( pushq, %rax );
		}
		//Fill relevant registers
		for ( int param=0; param<MIN(6,expr->children[1]->n_children); param++ ) {
		    printf ( "\tpopq\t%s\n", record[param] );
		}
		//Call function
	        printf ( "\tcall\t_%s\n", expr->children[0]->data );
		//Reset stack
	        if ( expr->children[1]->n_children > 6 ) {
		  printf ( "\taddq\t$%llu, %%rsp\n", 8*(expr->children[1]->n_children-6) );
		}
		//Reset registers
	        for ( int i = 5; i >= 0; i-- ) {
	            printf ( "\tpopq\t%s\n", record[i] );
	        }
	    }
	}
    }
}

static void
generate_assignment_statement ( node_t *statement, symbol_t *function )
{
    generate_expression ( statement->children[1], function );
    printf ( "\tmovq\t%%rax, " );
    generate_identifier ( statement->children[0], function );
    printf ( "\n" );
}

static void
generate_print_statement ( node_t *statement, symbol_t *function )
{
    for ( size_t i=0; i<statement->n_children; i++ )
    {
        node_t *item = statement->children[i];
        switch ( item->type )
        {
            case STRING_DATA:
                printf ( "\tmovq\t$STR%zu, %%rsi\n", *((size_t *)item->data) );
                ASM2 ( movq, $strout, %rdi );
                ASM1 ( call, printf );
                break;
            case NUMBER_DATA:
                printf ("\tmovq\t$%lld, %%rsi\n", *((int64_t *)item->data) );
                ASM2 ( movq, $intout, %rdi );
                ASM1 ( call, printf );
                break;
            case IDENTIFIER_DATA:
                printf ( "\tmovq\t" );
                generate_identifier ( item, function );
                printf ( ", %%rsi\n" );
                ASM2 ( movq, $intout, %rdi );
                ASM1 ( call, printf );
                break;
            case EXPRESSION:
	        generate_expression ( item, function );
                ASM2 ( movq, %rax, %rsi );
                ASM2 ( movq, $intout, %rdi );
                ASM1 ( call, printf );
                break;
        }
    }
    ASM2 ( movq, $'\n', %rdi );
    ASM1 ( call, putchar );
}
 
static void
generate_if_statement ( node_t *statement, symbol_t *function )
{
    condcnt++;
    size_t cond = condcnt;
    ASM1 ( pushq, %rdx );
    generate_expression ( statement->children[0]->children[0], function );
    ASM1 ( pushq, %rax );
    generate_expression ( statement->children[0]->children[1], function );
    ASM1 ( popq, %rdx );
    ASM2 ( cmpq, %rax, %rdx );
    ASM1 ( popq, %rdx );

    switch ( *((char *)statement->children[0]->data) ) {
        case '=':
	    if ( statement->n_children == 2 ) {
	        printf ( "\tjne\tENDIF%lu\n", cond );
	    } else {
	        printf ( "\tjne\tELSE%lu\n", cond );
	    }
	    break;
        case '<':
	  if ( statement->n_children == 2 ) {
	        printf ( "\tjge\tENDIF%lu\n", cond );
	    } else {
	        printf ( "\tjge\tELSE%lu\n", cond );
	    }
	    break;
        case '>':
	    if ( statement->n_children == 2 ) {
	        printf ( "\tjle\tENDIF%lu\n", cond );
	    } else {
	        printf ( "\tjle\tELSE%lu\n", cond );
	    }
	    break;
    }

    generate_node ( statement->children[1], function );
    if ( statement->n_children == 3 ) {
        printf ( "\tjmp\tENDIF%lu\n", cond );
        printf ( "ELSE%lu:\n", cond );
	generate_node ( statement->children[2], function );
    }
    printf ( "ENDIF%lu:\n", cond );
}

static void
generate_while_statement ( node_t *statement, symbol_t *function )
{
    loopcnt++;
    size_t loop = loopcnt;
    StackPush(loopstack, &loop);
    
    printf ( "WHILELOOP%lu:\n", loop );
    ASM1 ( pushq, %rdx );
    generate_expression ( statement->children[0]->children[0], function );
    ASM1 ( pushq, %rax );
    generate_expression ( statement->children[0]->children[1], function );
    ASM1 ( popq, %rdx );
    ASM2 ( cmpq, %rax, %rdx );
    ASM1 ( popq, %rdx );

    switch ( *((char *)statement->children[0]->data) ) {
        case '=':
	    printf ( "\tjne\tENDWHILE%lu\n", loop );
	    break;
        case '<':
	    printf ( "\tjge\tENDWHILE%lu\n", loop );
	    break;
        case '>':
	    printf ( "\tjle\tENDWHILE%lu\n", loop );
	    break;
    }
    
    generate_node ( statement->children[1], function );
    printf ( "\tjmp\tWHILELOOP%lu\n", loop );
    printf ( "ENDWHILE%lu:\n", loop );
    StackPop(loopstack, &loop);
}

static void
generate_node ( node_t *node, symbol_t *function )
{
    size_t loop;
    switch (node->type)
    {
        case WHILE_STATEMENT:
	    generate_while_statement ( node, function );
	    break;
        case NULL_STATEMENT:
            StackPop(loopstack, &loop);
	    printf ( "\tjmp\tWHILELOOP%lu\n", loop );
            StackPush(loopstack, &loop);
	    break;
        case IF_STATEMENT:
	    generate_if_statement ( node, function );
            break;
        case PRINT_STATEMENT:
	    generate_print_statement ( node, function );
            break;
        case ASSIGNMENT_STATEMENT:
	    generate_assignment_statement ( node, function );
            break;
        case RETURN_STATEMENT:
	    generate_expression ( node->children[0], function );
            ASM0 ( leave );
            ASM0 ( ret );
            break;
        default:
	    RECUR(node, function);
            break;
    }
}

static void
generate_function ( symbol_t *function )
{
    printf ( "_%s:\n", function->name );
    ASM1 ( pushq, %rbp );
    ASM2 ( movq, %rsp, %rbp );

    // Save arguments in local stack frame
    for ( size_t arg=0; arg<MIN(6,function->nparms); arg++ )
            printf ( "\tpushq\t%s\n", record[arg] );
    //Make space for local variables
    size_t n_locals = tlhash_size(function->locals) - function->nparms;
    printf ( "\tsubq\t$%ld, %%rsp\n", 8*n_locals );
    // Align stack to 16 bytes if odd # of locals/arguments
    if ( (tlhash_size(function->locals)&1) == 1 )
        puts ( "\tpushq\t$0" );
    generate_node ( function->node, function );
}

void
generate_program ( void )
{
    condcnt = 0;
    condstack = malloc ( sizeof(genstack_t) );
    StackNew ( condstack, sizeof(size_t) );
    loopcnt = 0;
    loopstack = malloc ( sizeof(genstack_t) );
    StackNew ( loopstack, sizeof(size_t) );
    
    size_t n_globals = tlhash_size(global_names);
    symbol_t *global_list[n_globals];
    tlhash_values ( global_names, (void **)&global_list );

    symbol_t *first_function;
    for ( size_t i=0; i<tlhash_size(global_names); i++ )
        if ( global_list[i]->type == SYM_FUNCTION && global_list[i]->seq == 0 )
        {
            first_function = global_list[i];
            break;
        }

    generate_stringtable();
    generate_global_variables();
    generate_main ( first_function );
    for ( size_t i=0; i<tlhash_size(global_names); i++ )
        if ( global_list[i]->type == SYM_FUNCTION )
            generate_function ( global_list[i] );

    StackDispose ( condstack );
    free ( condstack );
    StackDispose ( loopstack );
    free ( loopstack );
}
