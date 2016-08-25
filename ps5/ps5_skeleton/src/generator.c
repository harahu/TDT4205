#include "vslc.h"

#define MIN(a,b) (((a)<(b)) ? (a):(b))
static const char *record[6] = {
    "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
};

static void
generate_stringtable ( void )
{
    /* These can be used to emit numbers, strings and a run-time
     * error msg. from main
     */ 
    puts ( ".section .rodata" );
    puts ( "intout: .string \"\%ld \"" );
    puts ( "strout: .string \"\%s \"" );
    puts ( "errout: .string \"Wrong number of arguments\"" );

    for (int i = 0; i < stringc; i++) {
        printf ("STR%i:.string %s\n", i, string_list[i]);
    }
    printf( "\n" );
}

static void
generate_globvars ( size_t n_globals, symbol_t **global_list )
{
    puts ( ".section .data" );
    for ( size_t i=0; i<n_globals; i++ )
        if ( global_list[i]->type == SYM_GLOBAL_VAR )
          printf ( "_%s: .zero 8\n", global_list[i]->name);
    
    printf( "\n" );
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
print_call ( node_t *ident )
{
  symbol_t *sym = ident->entry;
  switch ( sym->type ) {
      case SYM_GLOBAL_VAR:
	  printf ( "_%s", ident->data );
	  break;
      case SYM_LOCAL_VAR:
	  //not implemented yet
	  break;
      case SYM_PARAMETER:
	  if ( sym->seq <= 5 ) {
	      printf ( "%ld(%%rbp)", -8*( ((long)sym->seq)+1) );
	  } else {
	      printf ( "%ld(%%rbp)", 8*(sym->seq-5) );
	  }  
	  break;
  }
}

static void
generate_subtree ( node_t *root )
{
    if ( root == NULL ) {
        return;
    }

    switch ( root->type ) {
        case EXPRESSION:
            if ( root->children[0]->type == NUMBER_DATA ) {
	      printf ( "\tpushq $%i\n", root->children[0]->data );
            }
            else if ( root->children[0]->type == IDENTIFIER_DATA ) {
	        printf ( "\tpushq " );
		print_call ( root->children[0] );
		printf ( "\n" );
            }
	    else if ( root->children[0]->type == EXPRESSION ) {
                generate_subtree ( root->children[0] );
	        puts ( "\tpushq %rax" );
            }
	    if ( root->n_children == 2 ) {
	        if ( root->children[1]->type == NUMBER_DATA ) {
		    printf ( "\tmovq $%i, %%rax", root->children[1]->data );
                }
                else if ( root->children[1]->type == IDENTIFIER_DATA ) {
		    printf ( "\tmovq " );
		    print_call ( root->children[1] );
		    printf ( ", %%rax\n" );
                }
	        else if ( root->children[1]->type == EXPRESSION ) {
		    generate_subtree ( root->children[0] );
                }
	        switch ( *((char *)root->data) ) {
                    case '+':
		        puts ( "\taddq %rax, (%rsp)" );
			puts ( "\tpopq %rax" );
		        break;
                    case '-':
		        puts ( "\tsubq %rax, (%rsp)" );
			puts ( "\tpopq %rax" );
		        break;
                    case '*':
		        puts ( "\tcqo" );
		        puts ( "\timulq (%rsp)" );
			puts ( "\tpopq %rdx" );
		        break;
                    case '/':
			puts ( "\tcqo" );
		        puts ( "\tidivq (%rsp)" );
			puts ( "\tpopq %rdx" );
		        break;
                }
		puts ( "\taddq $8, %rsp" );
	    }
	    else {
	        if ( *((char *)root->data) == '-' ) {
		  puts ( "\tpopq %rax" );
		  puts ( "\tnegq %rax" );
	        }
		else {
		    puts ( "\tERROR: Non-minus unary expression!" );
		}
	    }
	    break;
        case ASSIGNMENT_STATEMENT:
            switch ( root->children[1]->type ) {
	        case STRING_DATA:
	      	    printf ( "\tmovq $STR%zu, ", *((size_t *)root->children[1]->data) );
		    print_call ( root->children[0] );
		    printf ( "\n" );
		    break;
	        case IDENTIFIER_DATA:
		    printf ( "\tmovq " );
		    print_call ( root->children[1] );
		    printf ( ", %%rax\n" );
		    printf ( "\tmovq %%rax, " );
		    print_call ( root->children[0] );
		    printf ( "\n" );
		    break;
		case NUMBER_DATA:
		    printf ( "\tmovq $%i, ", root->children[1]->data );
		    print_call ( root->children[0] );
		    printf ( "\n" );
		  break;
	        case EXPRESSION:
		    generate_subtree ( root->children[1] );
		    printf ( "\tmovq %%rax, " );
		    print_call ( root->children[0] );
		    printf ( "\n" );
		    break;
            }
	    break;
        case PRINT_STATEMENT:
	    for ( uint64_t i=0; i<root->n_children; i++ ) {
	        switch ( root->children[i]->type ) {
		    case STRING_DATA:
		        //setup and call printf with strout and the string
		        printf ( "\tmovq $STR%zu, %%rsi\n", *((size_t *)root->children[i]->data) );
			puts ( "\tmovq $strout, %rdi" );
			puts ( "\tcall printf" );
		        break;
	            case IDENTIFIER_DATA:
		        //setup and call printf with intout and the contents of the ident. address
		        printf ( "\tmovq " );
			print_call ( root->children[i] );
		        printf ( ", %%rsi\n" );
			puts ( "\tmovq $intout, %rdi" );
			puts ( "\tcall printf" );
		        break;
		    case NUMBER_DATA:
		        //setup and call printf with intout and the number
		        printf ( "\tmovq $%i, %%rsi\n", root->children[i]->data );
			puts ( "\tmovq $intout, %rdi" );
			puts ( "\tcall printf" );
		        break;
	            case EXPRESSION:
		        //generate the expression, setup and call printf with intout and the contents of %rax
		        generate_subtree ( root->children[i] );
		        puts ( "\tmovq %rax, %rsi" );
			puts ( "\tmovq $intout, %rdi" );
			puts ( "\tcall printf" );
		        break;
	        }
	    }
	    //Output new line
	    puts ( "\tmovq $'\\n', %rdi" );
	    puts ( "\tcall putchar" );
	    break;
        case RETURN_STATEMENT:
	    switch ( root->children[0]->type ) {
	        case STRING_DATA:
		    printf ( "\tmovq $STR%zu, %rax\n", *((size_t *)root->children[0]->data) );
		    puts ( "\tleave" );
		    puts ( "\tret" );
		    break;
	        case IDENTIFIER_DATA:
		    printf ( "\tmovq $_%s, %rax\n", root->children[0]->data );
		    puts ( "\tleave" );
		    puts ( "\tret" );
		    break;
		case NUMBER_DATA:
		    printf ( "\tmovq $%i, %rax\n", root->children[0]->data );
		    puts ( "\tleave" );
		    puts ( "\tret" );
		    break;
	        case EXPRESSION:
		    generate_subtree ( root->children[0] );
		    puts ( "\tleave" );
		    puts ( "\tret" );
		    break;
            }
	    break;
        default:
	    for ( uint64_t i=0; i<root->n_children; i++ ) {
                generate_subtree ( root->children[i] );
	    }
	    break;
    }
}

static void
generate_func ( symbol_t *sym )
{
    printf ( "_%s:\n", sym->name );
    puts ( "\tpushq\t%rbp" );
    puts ( "\tmovq\t%rsp, %rbp" );
    
    for ( int arg=1; arg<=MIN(6,sym->nparms); arg++ ) {
        printf ( "\tpushq\t%s\n", record[arg-1] );
    }

    if ( MIN(6,sym->nparms) % 2 != 0 ) {
        puts ( "\tpushq\t$42" );
    }

    generate_subtree ( sym->node );
    
}

void
generate_program ( void )
{
    size_t n_globals = tlhash_size(global_names);
    symbol_t *global_list[n_globals];
    tlhash_values ( global_names, (void **)&global_list );

    generate_stringtable();
    generate_globvars( n_globals, &global_list );

    int mainfunc = 0;
    for ( size_t i=0; i<n_globals; i++ ) {
        if ( global_list[i]->type == SYM_FUNCTION ) {
	    if ( mainfunc == 0 ) {
	        generate_main( global_list[i] );
	        mainfunc = 1;
	    }
	    generate_func( global_list[i] );
	}
    }
}
