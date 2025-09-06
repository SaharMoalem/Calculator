# Calculator

A software calculator designed to evaluate mathematical expressions using a stack data structure and the Polish Notation (postfix) algorithm. It leverages lookup tables (LUTs) and a finite state machine (FSM) to parse input efficiently and ensure correct operator precedence.

---

## Prerequisites

- Linux OS
- C compiler (gcc)

---

## API

### `Calculate`

```c
status_t Calculate(const char* expr, double* ans);
```

**Description:**\
Evaluates a mathematical expression given as a string and stores the result in ans.

**Parameters:**

- `expr` — Input expression in standard infix notation (e.g., "3 + 5 * 2")
- `ans` — Pointer to a double where the result will be stored

**Returns:**

- SUCCESS on successful evaluation
- INVALID_SYNTAX if the expression is invalid
- MATH_ERROR on math errors (e.g., division by zero)
- FAILED_ALLOCATION if memory allocation of the stack fails

---

## Setup & Usage

### Build Instructions
in Calculator/bin, run the following -

```bash
gcc -ansi -pedantic-errors -Wall -Wextra -g ../test/test_calculator.c ../src/calculator.c ../ds/src/stack.c -I ../include/ -I ../ds/include/ -o calculator
```

Use test_calculator.c for testing or other files from projects of yours.

---

## How It Works

1. The FSM + LUTs parse each character in the expression.
2. Numbers are pushed onto a numbers stack, operators onto an operators stack.
3. Operator precedence is managed via the relations LUT.
4. When precedence rules dictate, operators are popped and executed using the operation LUT.
5. The process continues until the entire expression is evaluated.

---

## Main Components

- **stack**\
  Generic stack implementation used to manage numbers and operators during evaluation.

- **LUTs (Lookup Tables)**\
  * Transition LUT — guides the FSM between states (START, WAIT_FOR_OPERATOR, etc.).
  * Input LUT — classifies input characters (digit, operator, bracket, etc.).
  * Relations LUT — defines operator precedence and handling logic.
  * Operation LUT — maps operators (+, -, *, /, ^) to their respective execution functions.

- **FSM (Finite State Machine)**\
  Controls parsing flow and detects invalid syntax.

- **Handlers**\
  * Operators: Addition, subtraction (binary & unary), multiplication, division (with zero check), power.
  * Errors: Handles invalid syntax, math errors, and allocation failures.

---


