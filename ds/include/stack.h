#ifndef __STACK_H__
#define __STACK_H__

#include <stddef.h> /*size_t*/


typedef struct stack stack_t;

/* @Desc: Create and initialize a given stack_t type struct of certain element 
   @params: capacity of the stack, the size of the elements to insert to it
   @return value: pointer to the new created stack*/

stack_t* StackCreate(size_t capacity, size_t element_size);

/* @Desc: Free the given Stack from memory 
   @params: Pointer to the stack to destroy*/

void StackDestroy(stack_t* stack);

/* @Desc: Insert an element on top of stack 
   @params: Pointer to the stack, element to insert
   @return value: 1 if function succeeded, otherwise 0*/

int StackPush(stack_t* stack, const void* element);

/* @Desc: Remove an element on top of stack (doesn't save the popped element)
   @params: Pointer to the stack   */

void StackPop(stack_t* stack);

/* @Desc: Check if the stack is empty 
   @params: Pointer to the stack
   @return value: 1 if stack is empty, otherwise 0*/

int StackIsEmpty(const stack_t* stack);

/* @Desc: Get the value at the top of the stack 
   @params: Pointer to the stack, element to assign the value to*/

void StackPeek(const stack_t* stack, void* element);

/* @Desc: Get the capacity of the stack 
   @params: Pointer to the stack
   @return value: Capacity of the stack*/

size_t StackCapacity(const stack_t* stack);

/* @Desc: Get the current size of the stack 
   @params: Pointer to the stack
   @return value: Size of the stack*/
   
size_t StackSize(const stack_t* stack);

#endif  /*End of header guard*/ 
