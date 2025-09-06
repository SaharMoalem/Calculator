#include <string.h>  /* strlen */
#include <assert.h>  /* assert */
#include <ctype.h>   /* isspace */
#include <stdlib.h>  /* strtod */
#include <stddef.h>  /* size_t */

#include "calculator.h"
#include "stack.h"

#define ASCII_SIZE 255

typedef enum
{
    START,
    WAIT_FOR_OPERATOR,
    WAIT_FOR_NUMBER,
    ERROR,
    NUM_OF_STATE
} State;

typedef enum
{
    PLUS,
    UNARY_PLUS,
    MINUS,
    UNARY_MINUS,
    MULT,
    DIV,
    POWER,
    OPEN_BRACKETS,
    CLOSE_BRACKETS,
    OTHER,
    DIGIT,
    NUM_OF_INPUTS
} Input;

typedef status_t (*relations_handler)(Input, stack_t*, stack_t*);
typedef status_t (*action_func)(const char**, stack_t*, stack_t*);
typedef status_t (*operate_func)(stack_t*);
typedef status_t (*finalize_state_func)(double*, stack_t*, stack_t*);

static State transition_LUT[NUM_OF_STATE][NUM_OF_INPUTS];
static Input input_LUT[ASCII_SIZE];
static action_func action_funcs[NUM_OF_STATE][NUM_OF_STATE];
static relations_handler relations_LUT[NUM_OF_INPUTS][NUM_OF_INPUTS];
static operate_func operation_funcs_LUT[NUM_OF_INPUTS];
static Input repeat_input_LUT[NUM_OF_INPUTS];
static finalize_state_func finalize_state_LUT[NUM_OF_STATE];

static void InitTransitionLUT(void)
{
    size_t i = 0;

    transition_LUT[START][DIGIT] = WAIT_FOR_OPERATOR;
    transition_LUT[START][PLUS] = WAIT_FOR_NUMBER;
    transition_LUT[START][MINUS] = WAIT_FOR_NUMBER;
    transition_LUT[START][MULT] = ERROR;
    transition_LUT[START][DIV] = ERROR;
    transition_LUT[START][POWER] = ERROR;
    transition_LUT[START][OPEN_BRACKETS] = START;
    transition_LUT[START][CLOSE_BRACKETS] = ERROR;
    transition_LUT[START][OTHER] = ERROR;

    transition_LUT[WAIT_FOR_OPERATOR][DIGIT] = ERROR;
    transition_LUT[WAIT_FOR_OPERATOR][PLUS] = WAIT_FOR_NUMBER;
    transition_LUT[WAIT_FOR_OPERATOR][MINUS] = WAIT_FOR_NUMBER;
    transition_LUT[WAIT_FOR_OPERATOR][MULT] = WAIT_FOR_NUMBER;
    transition_LUT[WAIT_FOR_OPERATOR][DIV] = WAIT_FOR_NUMBER;
    transition_LUT[WAIT_FOR_OPERATOR][POWER] = WAIT_FOR_NUMBER;
    transition_LUT[WAIT_FOR_OPERATOR][OPEN_BRACKETS] = ERROR;
    transition_LUT[WAIT_FOR_OPERATOR][CLOSE_BRACKETS] = WAIT_FOR_OPERATOR;
    transition_LUT[WAIT_FOR_OPERATOR][OTHER] = ERROR;

    transition_LUT[WAIT_FOR_NUMBER][DIGIT] = WAIT_FOR_OPERATOR;
    transition_LUT[WAIT_FOR_NUMBER][PLUS] = WAIT_FOR_NUMBER;
    transition_LUT[WAIT_FOR_NUMBER][MINUS] = WAIT_FOR_NUMBER;
    transition_LUT[WAIT_FOR_NUMBER][MULT] = ERROR;
    transition_LUT[WAIT_FOR_NUMBER][DIV] = ERROR;
    transition_LUT[WAIT_FOR_NUMBER][POWER] = ERROR;
    transition_LUT[WAIT_FOR_NUMBER][OPEN_BRACKETS] = WAIT_FOR_NUMBER;
    transition_LUT[WAIT_FOR_NUMBER][CLOSE_BRACKETS] = ERROR;
    transition_LUT[WAIT_FOR_NUMBER][OTHER] = ERROR; 

    for( ; i < NUM_OF_INPUTS; ++i)
    {
        transition_LUT[ERROR][i] = ERROR;
    }
}

static void InitInputLUT(void)
{
    size_t i = 0;

    for( ; i < ASCII_SIZE; ++i)
    {
        input_LUT[i] = OTHER;
    }

    for(i = '0'; i <= '9'; ++i)
    {
        input_LUT[i] = DIGIT;
    }

    input_LUT['.'] = DIGIT;
    input_LUT['+'] = PLUS;
    input_LUT['-'] = MINUS;
    input_LUT['*'] = MULT;
    input_LUT['/'] = DIV;
    input_LUT['^'] = POWER;
    input_LUT['('] = OPEN_BRACKETS;
    input_LUT[')'] = CLOSE_BRACKETS;
}

static void InitRepeatInputLUT(void)
{
    size_t i = 0;

    for( ; i < NUM_OF_INPUTS; ++i)
    {
        repeat_input_LUT[i] = OTHER;
    }

    repeat_input_LUT[PLUS] = UNARY_PLUS;
    repeat_input_LUT[MINUS] = UNARY_MINUS;
    repeat_input_LUT[OPEN_BRACKETS] = OPEN_BRACKETS;
}

static void StackPeekPop(stack_t* stack, void* element)
{
    StackPeek(stack, element);
    StackPop(stack);
}

static status_t HandleReadingNumber(const char** str, stack_t* numbers,
                                                            stack_t* operators)
{
    double result = 0;
    char* runner;
    (void)operators;

    result = strtod(*str, &runner);
    StackPush(numbers, &result);
    *str = runner;

    return SUCCESS;
}

static status_t HandleReadingOperator(const char** str, stack_t* numbers,
                                                            stack_t* operators)
{
    Input stack_top;
    status_t status;
    Input curr_input = input_LUT[(unsigned char)**str];
    
    StackPeek(operators, &stack_top);
    status = relations_LUT[stack_top][curr_input](curr_input, numbers,
                                                                    operators);

    ++(*str);

    return status;
}

static status_t HandleRepeatNumber(const char** str, stack_t* numbers,
                                                            stack_t* operators)
{
    Input stack_top;
    Input curr_input = input_LUT[(unsigned char)**str];

    StackPeek(operators, &stack_top);
    ++(*str);

    return relations_LUT[stack_top][curr_input](curr_input, numbers, operators);
}

static status_t HandleRepeatOperator(const char** str, stack_t* numbers,
                                                            stack_t* operators)
{
    Input stack_top;
    Input curr_input = repeat_input_LUT[input_LUT[(unsigned char)**str]];

    StackPeek(operators, &stack_top);
    ++(*str);

    return relations_LUT[stack_top][curr_input](curr_input, numbers, operators);
}

static status_t HandleSyntaxError(const char** str, stack_t* numbers,
                                                            stack_t* operators)
{
    (void)str;
    (void)numbers;
    (void)operators;

    return INVALID_SYNTAX;
}

static void InitActionFuncs()
{
    action_funcs[START][START] = HandleReadingOperator;
    action_funcs[START][WAIT_FOR_OPERATOR] = HandleReadingNumber;
    action_funcs[START][WAIT_FOR_NUMBER] = HandleRepeatOperator;
    action_funcs[START][ERROR] = HandleSyntaxError;
    
    action_funcs[WAIT_FOR_OPERATOR][START] = HandleSyntaxError;
    action_funcs[WAIT_FOR_OPERATOR][WAIT_FOR_OPERATOR] = HandleRepeatNumber;
    action_funcs[WAIT_FOR_OPERATOR][WAIT_FOR_NUMBER] = HandleReadingOperator;
    action_funcs[WAIT_FOR_OPERATOR][ERROR] = HandleSyntaxError;

    action_funcs[WAIT_FOR_NUMBER][START] = HandleSyntaxError;
    action_funcs[WAIT_FOR_NUMBER][WAIT_FOR_OPERATOR] = HandleReadingNumber;
    action_funcs[WAIT_FOR_NUMBER][WAIT_FOR_NUMBER] = HandleRepeatOperator;
    action_funcs[WAIT_FOR_NUMBER][ERROR] = HandleSyntaxError;

    action_funcs[ERROR][START] = HandleSyntaxError;
    action_funcs[ERROR][WAIT_FOR_OPERATOR] = HandleSyntaxError;
    action_funcs[ERROR][WAIT_FOR_NUMBER] = HandleSyntaxError;
    action_funcs[ERROR][ERROR] = HandleSyntaxError;
}

static status_t HandleInvalidSyntax(Input input, stack_t* numbers,
                                                            stack_t* operators)
{
    (void)input;
    (void)numbers;
    (void)operators;

    return INVALID_SYNTAX;
}

static status_t HandlePushOperator(Input input, stack_t* numbers,
                                                            stack_t* operators)
{
    (void)numbers;
    StackPush(operators, &input);

    return SUCCESS;
}

static status_t HandlePopOperator(Input input, stack_t* numbers,
                                                            stack_t* operators)
{
    (void)input;
    (void)numbers;
    StackPop(operators);

    return SUCCESS;
}

static status_t HandleExecuteOldOperator(Input input, stack_t* numbers,
                                                            stack_t* operators)
{
    Input curr_input;
    status_t status;
    StackPeekPop(operators, &curr_input);
    status = operation_funcs_LUT[curr_input](numbers);
    StackPeek(operators, &curr_input);
    return status == SUCCESS ? relations_LUT[curr_input][input](input, numbers,
                                                            operators) : status;
}

static void InitRelationshipLUT(void)
{
    size_t i = 0;
    size_t j = 0;

    for( ; i < NUM_OF_INPUTS; ++i)
    {
        for(j = 0; j < NUM_OF_INPUTS; ++j)
        {
            relations_LUT[i][j] = HandleInvalidSyntax;
        }
    }

    relations_LUT[OTHER][PLUS] = HandlePushOperator;
    relations_LUT[OTHER][UNARY_PLUS] = HandlePushOperator;
    relations_LUT[OTHER][MINUS] = HandlePushOperator;
    relations_LUT[OTHER][UNARY_MINUS] = HandlePushOperator;
    relations_LUT[OTHER][MULT] = HandlePushOperator;
    relations_LUT[OTHER][DIV] = HandlePushOperator;
    relations_LUT[OTHER][POWER] = HandlePushOperator;
    relations_LUT[OTHER][OPEN_BRACKETS] = HandlePushOperator;

    relations_LUT[PLUS][PLUS] = HandleExecuteOldOperator;
    relations_LUT[PLUS][UNARY_PLUS] = HandlePushOperator;
    relations_LUT[PLUS][MINUS] = HandleExecuteOldOperator;
    relations_LUT[PLUS][UNARY_MINUS] = HandlePushOperator;
    relations_LUT[PLUS][MULT] = HandlePushOperator;
    relations_LUT[PLUS][DIV] = HandlePushOperator;
    relations_LUT[PLUS][POWER] = HandlePushOperator;
    relations_LUT[PLUS][OPEN_BRACKETS] = HandlePushOperator;
    relations_LUT[PLUS][CLOSE_BRACKETS] = HandleExecuteOldOperator;

    relations_LUT[UNARY_PLUS][PLUS] = HandleExecuteOldOperator;
    relations_LUT[UNARY_PLUS][UNARY_PLUS] = HandlePushOperator;
    relations_LUT[UNARY_PLUS][MINUS] = HandleExecuteOldOperator;
    relations_LUT[UNARY_PLUS][UNARY_MINUS] = HandlePushOperator;
    relations_LUT[UNARY_PLUS][MULT] = HandleExecuteOldOperator;
    relations_LUT[UNARY_PLUS][DIV] = HandleExecuteOldOperator;
    relations_LUT[UNARY_PLUS][POWER] = HandlePushOperator;
    relations_LUT[UNARY_PLUS][OPEN_BRACKETS] = HandlePushOperator;
    relations_LUT[UNARY_PLUS][CLOSE_BRACKETS] = HandleExecuteOldOperator;

    relations_LUT[MINUS][PLUS] = HandleExecuteOldOperator;
    relations_LUT[MINUS][UNARY_PLUS] = HandlePushOperator;
    relations_LUT[MINUS][MINUS] = HandleExecuteOldOperator;
    relations_LUT[MINUS][UNARY_MINUS] = HandlePushOperator;
    relations_LUT[MINUS][MULT] = HandlePushOperator;
    relations_LUT[MINUS][DIV] = HandlePushOperator;
    relations_LUT[MINUS][POWER] = HandlePushOperator;
    relations_LUT[MINUS][OPEN_BRACKETS] = HandlePushOperator;
    relations_LUT[MINUS][CLOSE_BRACKETS] = HandleExecuteOldOperator;

    relations_LUT[UNARY_MINUS][PLUS] = HandleExecuteOldOperator;
    relations_LUT[UNARY_MINUS][UNARY_PLUS] = HandlePushOperator;
    relations_LUT[UNARY_MINUS][MINUS] = HandleExecuteOldOperator;
    relations_LUT[UNARY_MINUS][UNARY_MINUS] = HandlePushOperator;
    relations_LUT[UNARY_MINUS][MULT] = HandleExecuteOldOperator;
    relations_LUT[UNARY_MINUS][DIV] = HandleExecuteOldOperator;
    relations_LUT[UNARY_MINUS][POWER] = HandlePushOperator;
    relations_LUT[UNARY_MINUS][OPEN_BRACKETS] = HandlePushOperator;
    relations_LUT[UNARY_MINUS][CLOSE_BRACKETS] = HandleExecuteOldOperator;

    relations_LUT[MULT][PLUS] = HandleExecuteOldOperator;
    relations_LUT[MULT][UNARY_PLUS] = HandlePushOperator;
    relations_LUT[MULT][MINUS] = HandleExecuteOldOperator;
    relations_LUT[MULT][UNARY_MINUS] = HandlePushOperator;
    relations_LUT[MULT][MULT] = HandleExecuteOldOperator;
    relations_LUT[MULT][DIV] = HandleExecuteOldOperator;
    relations_LUT[MULT][POWER] = HandlePushOperator;
    relations_LUT[MULT][OPEN_BRACKETS] = HandlePushOperator;
    relations_LUT[MULT][CLOSE_BRACKETS] = HandleExecuteOldOperator;

    relations_LUT[DIV][PLUS] = HandleExecuteOldOperator;
    relations_LUT[DIV][UNARY_PLUS] = HandlePushOperator;
    relations_LUT[DIV][MINUS] = HandleExecuteOldOperator;
    relations_LUT[DIV][UNARY_MINUS] = HandlePushOperator;
    relations_LUT[DIV][MULT] = HandleExecuteOldOperator;
    relations_LUT[DIV][DIV] = HandleExecuteOldOperator;
    relations_LUT[DIV][POWER] = HandlePushOperator;
    relations_LUT[DIV][OPEN_BRACKETS] = HandlePushOperator;
    relations_LUT[DIV][CLOSE_BRACKETS] = HandleExecuteOldOperator;

    relations_LUT[POWER][PLUS] = HandleExecuteOldOperator;
    relations_LUT[POWER][UNARY_PLUS] = HandlePushOperator;
    relations_LUT[POWER][MINUS] = HandleExecuteOldOperator;
    relations_LUT[POWER][UNARY_MINUS] = HandlePushOperator;
    relations_LUT[POWER][MULT] = HandleExecuteOldOperator;
    relations_LUT[POWER][DIV] = HandleExecuteOldOperator;
    relations_LUT[POWER][POWER] = HandleExecuteOldOperator;
    relations_LUT[POWER][OPEN_BRACKETS] = HandlePushOperator;
    relations_LUT[POWER][CLOSE_BRACKETS] = HandleExecuteOldOperator;

    relations_LUT[OPEN_BRACKETS][PLUS] = HandlePushOperator;
    relations_LUT[OPEN_BRACKETS][UNARY_PLUS] = HandlePushOperator;
    relations_LUT[OPEN_BRACKETS][MINUS] = HandlePushOperator;
    relations_LUT[OPEN_BRACKETS][UNARY_MINUS] = HandlePushOperator;
    relations_LUT[OPEN_BRACKETS][MULT] = HandlePushOperator;
    relations_LUT[OPEN_BRACKETS][DIV] = HandlePushOperator;
    relations_LUT[OPEN_BRACKETS][POWER] = HandlePushOperator;
    relations_LUT[OPEN_BRACKETS][OPEN_BRACKETS] = HandlePushOperator;
    relations_LUT[OPEN_BRACKETS][CLOSE_BRACKETS] = HandlePopOperator;
}

static status_t AddHandler(stack_t* numbers)
{
    double num1 = 0;
    double num2 = 0;

    StackPeekPop(numbers, &num1);
    StackPeekPop(numbers, &num2);
    num2 += num1;
    StackPush(numbers, &num2);

    return SUCCESS;
}

static status_t UnaryPlusHandler(stack_t* numbers)
{
    (void)numbers;
    return SUCCESS;
}

static status_t SubHandler(stack_t* numbers)
{
    double num1 = 0;
    double num2 = 0;

    StackPeekPop(numbers, &num1);
    StackPeekPop(numbers, &num2);
    num2 -= num1;
    StackPush(numbers, &num2);

    return SUCCESS;
}

static status_t UnaryMinusHandler(stack_t* numbers)
{
    double num = 0;

	StackPeekPop(numbers, &num);
	num *= -1;
	StackPush(numbers, &num);

	return SUCCESS;
}

static status_t MultiplyHandler(stack_t* numbers)
{
    double num1 = 0;
    double num2 = 0;

    StackPeekPop(numbers, &num1);
    StackPeekPop(numbers, &num2);
    num2 *= num1;
    StackPush(numbers, &num2);

    return SUCCESS;
}

static status_t DivideHandler(stack_t* numbers)
{
    double num1 = 0;
    double num2 = 0;

    StackPeekPop(numbers, &num1);
    StackPeekPop(numbers, &num2);

    if(num1 == 0)
    {
        return MATH_ERROR;
    }

    num2 /= num1;
    StackPush(numbers, &num2);

    return SUCCESS;
}

static status_t PowerHandler(stack_t* numbers)
{
    double num1 = 0;
    double num2 = 0;
    double result = 1;
    size_t i = 0;

    StackPeekPop(numbers, &num1);
    StackPeekPop(numbers, &num2);

    if(num1 < 0 && num2 == 0)
    {
        return MATH_ERROR;
    }

    num2 = num1 < 0 ? 1 / num2 : num2;
	num1 = num1 < 0 ? -num1 : num1;

    for( ; i < num1; ++i)
    {
        result *= num2;
    }

    StackPush(numbers, &result);

    return SUCCESS;
}

static status_t OpenBracketErrorHandler(stack_t* numbers)
{
    (void)numbers;

    return INVALID_SYNTAX;
}

static void InitOperationFuncs()
{
    operation_funcs_LUT[PLUS] = AddHandler;
    operation_funcs_LUT[UNARY_PLUS] = UnaryPlusHandler;
    operation_funcs_LUT[MINUS] = SubHandler;
    operation_funcs_LUT[UNARY_MINUS] = UnaryMinusHandler;
    operation_funcs_LUT[MULT] = MultiplyHandler;
    operation_funcs_LUT[DIV] = DivideHandler;
    operation_funcs_LUT[POWER] = PowerHandler;
    operation_funcs_LUT[OPEN_BRACKETS] = OpenBracketErrorHandler;
}

static status_t FsmReject(double* ans, stack_t* numbers, stack_t* operators)
{
    (void)ans;
    (void)numbers;
    (void)operators;
    return INVALID_SYNTAX;
}

static status_t CalculateAll(double* ans, stack_t* numbers, stack_t* operators)
{
    status_t status = SUCCESS;
    Input curr_input;

    StackPeek(operators, &curr_input);

    while(status == SUCCESS && curr_input != OTHER)
    {
        status = operation_funcs_LUT[curr_input](numbers);
        if(status == MATH_ERROR)
        {
            return MATH_ERROR;
        }

        StackPop(operators);
        StackPeek(operators, &curr_input);
    }

    StackPeek(numbers, ans);
    
    return status;
}

static void InitFinalizeLUT()
{
    finalize_state_LUT[START] = FsmReject;
    finalize_state_LUT[WAIT_FOR_OPERATOR] = CalculateAll;
    finalize_state_LUT[WAIT_FOR_NUMBER] = FsmReject;
    finalize_state_LUT[ERROR] = FsmReject;
}

static void InitLUTs(void)
{
    InitTransitionLUT();
    InitInputLUT();
    InitActionFuncs();
    InitRelationshipLUT();
    InitOperationFuncs();
    InitFinalizeLUT();
    InitRepeatInputLUT();
}

status_t Calculate(const char* str, double* ans)
{
    stack_t* numbers_stack = NULL;
    stack_t* operators_stack = NULL;
    State state = START;
    State prev_state;
    status_t status = SUCCESS;
    Input input = OTHER;

    assert(str);
    assert(ans);

    numbers_stack = StackCreate(strlen(str), sizeof(double));
    if(numbers_stack == NULL)
    {
        return FAILED_ALLOCATION;
    }

    operators_stack = StackCreate(strlen(str), sizeof(Input));
    if(operators_stack == NULL)
    {
        StackDestroy(numbers_stack);
        return FAILED_ALLOCATION;
    }

    InitLUTs();
    StackPush(operators_stack, &input);

    while(isspace(*str))
    {
        ++str;
    }

    while(*str != '\0' && state != ERROR && status != MATH_ERROR)
    {
        prev_state = state;
        input = input_LUT[(unsigned char)*str];
        state = transition_LUT[state][input];
        status = action_funcs[prev_state][state](&str, numbers_stack,
                                                            operators_stack);

        while(isspace(*str))
        {
            ++str;
        }
    }

    status = status == SUCCESS ? finalize_state_LUT[state](ans, numbers_stack,
                                            operators_stack) : status;
    StackDestroy(numbers_stack);
    StackDestroy(operators_stack);

    return status;
}