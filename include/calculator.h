#ifndef __CALCULATOR_H__
#define __CALCULATOR_H__

typedef enum
{
    SUCCESS = 0,
    MATH_ERROR = 1,
    INVALID_SYNTAX = 2,
    FAILED_ALLOCATION = 3
} status_t;


status_t Calculate(const char* str, double* ans);

#endif      /* calculator.h */