#ifndef KARG_TYPES_H
#define KARG_TYPES_H

/*
 * Convenience helpers for retrieving parsed CLI values as typed data.
 *
 * These functions build on the core argument parser and provide a simple
 * interface for consuming command-line input as integers, floats, strings,
 * or file handles.
 */

#include <stdio.h>

/* Retrieve a command-line argument as an integer value. */
int karg_getPramInt(char *label);
/* Retrieve a command-line argument as a floating-point value. */
float karg_getPramFloat(char *label);
/* Retrieve a command-line argument as a string value. */
char *karg_getPramString(char *label);
/* Retrieve a command-line argument as an opened file stream. */
FILE *karg_getPramFile(char *label, const char *mode);

#endif // KARG_TYPES_H