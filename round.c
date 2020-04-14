
// based in https://www.codeproject.com/Articles/58289/C-Round-Function

#include <stdint.h>
// input never is negative here then
// we not verify number negative for more performance

// examples:
// input 1.1 - output 1
// input 1.4 - output 1
// input 1.5 - output 2
// input 1.9 - output 2
uint64_t m_round(double number)
{
    return (uint64_t)(number + 0.5);
}
