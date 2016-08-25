#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "pencil.h"

int trans_table[13][11] =
{
//   a  d  e  m  n  o  r  t  u  v  w
	{0, 1, 0, 3, 0, 0, 0, 2, 0, 0, 0},
	{0, 1, 0, 3, 0, 0, 4, 2, 0, 0, 0},
	{0, 1, 0, 3, 0, 0, 0, 2, 5, 0, 0},
	{0, 1, 0, 3, 0, 6, 0, 2, 0, 0, 0},
	{7, 1, 0, 3, 0, 0, 0, 2, 0, 0, 0},
	{0, 1, 0, 3, 0, 0, 8, 2, 0, 0, 0},
	{0, 1, 0, 3, 0, 0, 0, 2, 0, 9, 0},
	{0, 1, 0, 3, 0, 0, 0, 2, 0, 0, 10},
	{0, 1, 0, 3, 11, 0, 0, 2, 0, 0, 0},
	{0, 1, 12, 3, 0, 0, 0, 2, 0, 0, 0},
	{0, 1, 0, 3, 0, 0, 0, 2, 0, 0, 0},
	{0, 1, 0, 3, 0, 0, 0, 2, 0, 0, 0},
	{0, 1, 0, 3, 0, 0, 0, 2, 0, 0, 0}
};

/*
 * This function is called before anything else, to initialize the
 * state machine. It is certainly possible to create implementations
 * which don't require any initialization, so just leave this blank if
 * you don't need it.
 */
void
init_transtab ( void )
{
	 
}

int
change_state ( int state, int next_char )
{
	char clean_char = tolower ( ( char ) next_char );
	switch ( clean_char )
	{
		case 'a': return trans_table[state][0]; break;
		case 'd': return trans_table[state][1]; break;
		case 'e': return trans_table[state][2]; break;
		case 'm': return trans_table[state][3]; break;
		case 'n': return trans_table[state][4]; break;
		case 'o': return trans_table[state][5]; break;
		case 'r': return trans_table[state][6]; break;
		case 't': return trans_table[state][7]; break;
		case 'u': return trans_table[state][8]; break;
		case 'v': return trans_table[state][9]; break;
		case 'w': return trans_table[state][10]; break;
		//all other chars
        default:
			return 0;
	    	break;
	}
}


/*
 * Return the next token from reading the given input stream.
 * The words to be recognized are 'turn', 'draw' and 'move',
 * while the returned tokens may be TURN, DRAW, MOVE or END (as
 * enumerated in 'pencil.h').
 */
command_t
next ( FILE *input )
{
	int state = 0;
	int next_char = fgetc ( input );
	while ( next_char != EOF ) 
	{
		state = change_state ( state, next_char );
		switch ( state )
		{
			case 10: return DRAW; break;
			case 11: return TURN; break;
		    case 12: return MOVE; break;
	        default:
		        break;
		}
		next_char = fgetc ( input );
	}
    return END;
}
