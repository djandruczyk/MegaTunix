/*
 * name: crx
 * description: Regular Expression Engine (light weight version) for C Language, using double-recursion and function pointers.
 * author: ken (hexabox) seto
 * date: 2009.08~09
 * license: BSD, GPL
 * version: 0.13.13
 *
 * usage:
 *     int   len;
 *     char  output[24];
 *     strncpy(output, regex("\\d*", "abc123", &len), len);
 *     output[len] = 0x00;
 */

#include <stdio.h>

#ifndef __CRX_H
#define __CRX_H

typedef enum {false, true} bool;

char* regex(char* pattern, char* string, int* found_len);

#endif
