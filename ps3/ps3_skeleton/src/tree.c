#include <vslc.h>


void
node_print ( node_t *root, int nesting )
{
    if ( root != NULL )
    {
        printf ( "%*c%s", nesting, ' ', node_string[root->type] );
        if ( root->type == IDENTIFIER_DATA ||
             root->type == STRING_DATA ||
             root->type == RELATION ||
             root->type == EXPRESSION ) 
            printf ( "(%s)", (char *) root->data );
        else if ( root->type == NUMBER_DATA )
            printf ( "(%lld)", *((int64_t *)root->data) );
        putchar ( '\n' );
        for ( int64_t i=0; i<root->n_children; i++ )
            node_print ( root->children[i], nesting+1 );
    }
    else
        printf ( "%*c%p\n", nesting, ' ', root );
}


void
node_init (node_t *nd, node_index_t type, void *data, uint64_t n_children, ...)
{
    va_list child_list;
    *nd = (node_t) {
        .type = type,
        .data = data,
        .entry = NULL,
        .n_children = n_children,
        .children = (node_t **) malloc ( n_children * sizeof(node_t *) )
    };
    va_start ( child_list, n_children );
    for ( uint64_t i=0; i<n_children; i++ )
        nd->children[i] = va_arg ( child_list, node_t * );
    va_end ( child_list );
}


void
node_finalize ( node_t *discard )
{
    if ( discard != NULL )
    {
		if ( discard->data != NULL )
		{
	        free ( discard->data );
			discard->data = NULL;	
		}
		
		if ( discard->children != NULL )
		{
	        free ( discard->children );
			discard->children = NULL;
		}
        
        free ( discard );
		discard = NULL;
    }
}


void
destroy_subtree ( node_t *discard )
{
    if ( discard != NULL )
    {
        for ( uint64_t i=0; i<discard->n_children; i++ ) {
            destroy_subtree ( discard->children[i] );
		}
        node_finalize ( discard );
    }
}

//Helper function to determine if a node is a list
int
is_list ( node_t *candidate )
{
	switch ( candidate->type ) 
	{
		case GLOBAL_LIST: return 1; break;
		case STATEMENT_LIST: return 1; break;
		case PRINT_LIST: return 1; break;
		case EXPRESSION_LIST: return 1; break;
		case VARIABLE_LIST: return 1; break;
		case ARGUMENT_LIST: return 1; break;
		case PARAMETER_LIST: return 1; break;
		case DECLARATION_LIST: return 1; break;
		default: return 0; break;
	}
}

//Helper function for cutting singular nodes out of the tree
void
remove_no_sibling_node ( node_t *node, node_t *parent )
{
	parent->n_children = node->n_children;
	free(parent->children);
	parent->children = node->children;
	node->children = NULL;
	node_finalize ( node );
}

void
simplify_tree ( node_t **simplified, node_t *root )
{	
	//Recursively iterate through the tree
	for ( uint64_t i=0; i<root->n_children; i++ ) {
		if (root->children[i] != NULL) {
			simplify_tree ( &root->children[i], root->children[i] );
		}
	}
	
	//Compacting lists
	if ( (is_list(root) == 1) && (root->n_children >= 1) ) {
		if ( root->children[0]->type == root->type ) {
			node_t *list_child = root->children[0];
			node_t **new_array = (node_t **) realloc ( list_child->children, (list_child->n_children+1)*sizeof(node_t*) );
			if(new_array != NULL) {
				list_child->children = NULL;
				new_array[list_child->n_children] = root->children[1];
				root->n_children = list_child->n_children+1;
				root->children = new_array;
				node_finalize ( list_child );
			}
			else {
				printf("Can't reallocate memory!\n");
			}
		}
	}
	
	//Remove printlists and items
	if (root->type == PRINT_STATEMENT) {
		//remove list
		remove_no_sibling_node ( root->children[0], root );
		//remove items
		for ( uint64_t i=0; i<root->n_children; i++ ) {
			node_t *child = root->children[i];
			root->children[i] = child->children[0];
			node_finalize ( child );
		}
	}
	
	//Remove globals
	if (root->type == PROGRAM) {
		for ( uint64_t i=0; i<root->children[0]->n_children; i++ ) {
			node_t *child = root->children[0]->children[i];
			root->children[0]->children[i] = child->children[0];
			node_finalize ( child );
		}
	}
	
	//Remove parameter_lists
	if ( root->type == PARAMETER_LIST ) {
		node_t *child = root->children[0];
		node_finalize( root );
		root = child;
	}
	
	//Removing argument_lists
	if ( root->type == ARGUMENT_LIST ) {
		node_t *child = root->children[0];
		node_finalize( root );
		root = child;
	}
	
	//Remove statements
	if ( root->type == STATEMENT ) {
		node_t *child = root->children[0];
		node_finalize( root );
		root = child;
	}
	
	//Compacting expressions
	if (root->type == EXPRESSION) {
		int concat = 1;
		for ( uint64_t i=0; i<root->n_children; i++ ) {
			if (root->children[i]->type != NUMBER_DATA) {
				concat = 0;
			}
		}
		if (concat == 1) {
			//determine new content
	        int64_t *value = malloc ( sizeof(int64_t) );
			if (root->data == NULL) {
				*value = * ((long*) (root->children[0]->data));
			}
			else if (strcmp(root->data, "+") == 0) {
				*value = *(long*)(root->children[0]->data) + *(long*)(root->children[1]->data);
			}
			else if (strcmp(root->data, "*") == 0) {
				*value = *(long*)(root->children[0]->data) * *(long*)(root->children[1]->data);
			}
			else if (strcmp(root->data, "/") == 0) {
				*value = *(long*)(root->children[0]->data) / *(long*)(root->children[1]->data);
			}
			else if (strcmp(root->data, "-") == 0) {
				if (root->n_children == 1) {
					*value = - *(long*)(root->children[0]->data);
				}
				else {
					*value = *(long*)(root->children[0]->data) - *(long*)(root->children[1]->data);
				}
			}
			//morph node and remove children
	        for ( uint64_t i=0; i<root->n_children; i++ ) {
	            destroy_subtree ( root->children[i] );
			}
			free ( root->children );
			root->children = NULL;
			free ( root->data );
			root->n_children = 0;
			root->data = value;
			root->type = NUMBER_DATA;
		}
		if ( (root->n_children == 1) && (root->data == NULL) && (root->children[0]->type == IDENTIFIER_DATA) ) {
			node_t *child = root->children[0];
			node_finalize( root );
			root = child;
		}
	}
    *simplified = root;
}
