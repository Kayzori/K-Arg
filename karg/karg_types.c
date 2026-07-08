/*
 * Typed helpers for consuming parsed command-line arguments.
 *
 * These utilities bridge the core parser API with common C data types,
 * making it easier for applications to read values in a predictable way.
 */

#include "karg_types.h"

#include "karg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int karg_getPramInt(char *label) {
    char *value = karg_getParam(label);
    return value ? atoi(value) : 0;
}

float karg_getPramFloat(char *label) {
    char *value = karg_getParam(label);
    return value ? (float)atof(value) : 0.0f;
}

char *karg_getPramString(char *label) {
    return karg_getParam(label);
}

char karg_isAbsolutePath(const char *path) {
    if (path == NULL || path[0] == '\0') {
        return 0;
    }

    if (path[0] == '/') {
        return 1;
    }

    if (path[0] == '\\') {
        return 1;
    }
    if (((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z'))
        && path[1] == ':'
        && (path[2] == '\\' || path[2] == '/')) {
        return 1;
    }

    return 0;
}

FILE *karg_getPramFile(char *label, const char *mode) {
    char *path = karg_getParam(label);

    if (path == NULL) {
        printf("[karg_types] <karg_getPramFile> argument \"%s\" was not provided\n", label);
        return NULL;
    }

    if (karg_isAbsolutePath(path)) {
        printf("[karg_types] <karg_getPramFile> \"%s\" resolved as absolute path: %s\n", label, path);
    } else {
        printf("[karg_types] <karg_getPramFile> \"%s\" resolved as relative path: %s\n", label, path);
    }

    FILE *file = fopen(path, mode);
    if (file == NULL) {
        printf("[karg_types] <karg_getPramFile> failed to open file: %s\n", path);
        return NULL;
    }

    return file;
}