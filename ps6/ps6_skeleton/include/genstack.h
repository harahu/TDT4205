/*Based on a general stack implementation outlined in a Stanford University lecture.
*Available at URL:
*https://see.stanford.edu/materials/icsppcs107/stack-implementation.pdf
*/

#include <stdbool.h>

typedef struct {
	void *elems;
	size_t elemSize;
	int logLength;
	int allocLength;
} genstack_t;

void StackNew (genstack_t *s, size_t elemSize);
void StackDispose (genstack_t *s);
bool StackEmpty (const genstack_t *s);
void StackPush (genstack_t *s, const void *elemAddr);
void StackPop (genstack_t *s, void *elemAddr);