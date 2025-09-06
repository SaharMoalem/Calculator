#include <stdlib.h>                  /* malloc, free */
#include <assert.h>                  /* assert */
#include <string.h>                  /* memcpy */

#include "stack.h"

#define SUCCESS (1)
#define FAILURE (0)

struct stack {
    size_t size;
    size_t capacity;
    size_t element_size;
};

stack_t* StackCreate(size_t capacity, size_t element_size)
{
    stack_t* p_stack = (stack_t*)malloc(sizeof(stack_t) + (capacity * element_size));

    if (NULL == p_stack)
    {
        return NULL;
    }

    p_stack->size = 0;
    p_stack->capacity = capacity;
    p_stack->element_size = element_size;

    return p_stack;
}

void StackDestroy(stack_t* stack)
{
    assert(NULL != stack);

    free(stack);
    stack = NULL;
}

int StackIsEmpty(const stack_t* stack)
{
    assert(NULL != stack);

    return 0 == stack->size;
}

void StackPeek(const stack_t* stack, void* element)
{
    unsigned char* p_stack = (unsigned char*)(stack + 1);
    
    assert(NULL != stack);
    assert(0 < stack->size);

    memcpy(element, p_stack + ((stack->size - 1) * stack->element_size), stack->element_size);
}

size_t StackCapacity(const stack_t* stack)
{
    assert(NULL != stack);

    return stack->capacity;
}

size_t StackSize(const stack_t* stack)
{
    assert(NULL != stack);

    return stack->size;
}

int StackPush(stack_t* stack, const void* element)
{
    unsigned char* p_stack = (unsigned char*)(stack + 1);
    
    assert(NULL != stack);

    if (stack->size == stack->capacity)
    {
        return FAILURE;
    }

    memcpy(p_stack + (stack->size * stack->element_size), element, stack->element_size);
    stack->size++;

    return SUCCESS;
}

void StackPop(stack_t* stack)
{
    assert(NULL != stack);
    
    stack->size--;
}