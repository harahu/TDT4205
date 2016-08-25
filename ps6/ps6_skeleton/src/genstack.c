/*Based on a general stack implementation outlined in a Stanford University lecture.
*Available at URL:
*https://see.stanford.edu/materials/icsppcs107/stack-implementation.pdf
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "genstack.h"

#define kInitialAllocationSize 4

void
StackNew (genstack_t *s, size_t elemSize)
{
	assert(elemSize > 0);
	s->elemSize = elemSize;
	s->logLength = 0;
	s->allocLength = kInitialAllocationSize;
	s->elems = malloc(kInitialAllocationSize * elemSize);
	assert(s->elems != NULL);
}

void
StackDispose (genstack_t *s)
{
	free(s->elems);
}

bool
StackEmpty (const genstack_t *s)
{
	return s->logLength == 0;
}

void
StackPush (genstack_t *s, const void *elemAddr)
{
	void *destAddr;
	if (s->logLength == s->allocLength) {
		s->allocLength *= 2;
		s->elems = realloc(s->elems, s->allocLength * s->elemSize);
		assert(s->elems != NULL);
	}
	
	destAddr = (char*) s->elems + s->logLength * s->elemSize;
	memcpy(destAddr, elemAddr, s->elemSize);
	s->logLength++;
}

void
StackPop (genstack_t *s, void *elemAddr)
{
	const void *sourceAddr;
	
	assert(!StackEmpty(s));
	s->logLength--;
	sourceAddr = (const char*)s->elems + s->logLength * s->elemSize;
	memcpy(elemAddr, sourceAddr, s->elemSize);
}