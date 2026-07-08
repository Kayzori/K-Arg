/*
 * Implementation of the karg command-line argument parser.
 *
 * The parser maintains a lightweight registry of arguments, supports
 * nested parent/child structures, validates required input, and exposes
 * a simple API for retrieving parsed values at runtime.
 */

#include "karg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KARG_MAX 255
#define ARG(id) (&args[(id) - 1])

typedef enum karg_type_t {
    KARG_TYPE_PARAM,
    KARG_TYPE_FLAG,
    KARG_TYPE_PARENT
} karg_type_t;

typedef struct karg_t {
    char *labels;
    char *description;
    char required;
    karg_type_t type;

    char *data;      // PARAM
    char flagSet;    // FLAG
    char active;     // PARENT: was it matched on the CLI

    argid_t *children;
    int childrenCount;
    int childrenCapacity;

    char isHelp;
} karg_t;

static karg_t args[KARG_MAX];
static int argsCount;

static argid_t roots[KARG_MAX];
static int rootsCount;

static argid_t helpParentId; // 0 = not set

static char karg_labelMatches(char *labels, char *target) {
    // if this matching logic ever becomes unreliable, revisit label parsing and token splitting.
    char *copy = strdup(labels);
    if (copy == NULL) {
        return 0;
    }

    char found = 0;
    char *saveptr = NULL;
    char *token = strtok_r(copy, " ", &saveptr);
    while (token != NULL) {
        if (strcmp(token, target) == 0) {
            found = 1;
            break;
        }
        token = strtok_r(NULL, " ", &saveptr);
    }

    free(copy);
    return found;
}

static argid_t karg_findInScope(argid_t *scope, int scopeCount, char *label) {
    for (int s = 0; s < scopeCount; s++) {
        if (karg_labelMatches(ARG(scope[s])->labels, label)) {
            return scope[s];
        }
    }
    return 0;
}

static argid_t karg_findByPath(char *path) {
    char *copy = strdup(path);
    if (copy == NULL) {
        return 0;
    }

    argid_t *scope = roots;
    int scopeCount = rootsCount;
    argid_t matchId = 0;

    char *saveptr = NULL;
    char *token = strtok_r(copy, " ", &saveptr);
    while (token != NULL) {
        matchId = karg_findInScope(scope, scopeCount, token);
        if (matchId == 0) {
            free(copy);
            return 0;
        }

        char *next = strtok_r(NULL, " ", &saveptr);
        if (next == NULL) {
            break; // last token in path — matchId is the answer
        }

        if (ARG(matchId)->type != KARG_TYPE_PARENT) {
            // path continues but current match isn't a parent — dead end
            free(copy);
            return 0;
        }

        scope = ARG(matchId)->children;
        scopeCount = ARG(matchId)->childrenCount;
        token = next;
    }

    free(copy);
    return matchId;
}

static argid_t karg_registerRaw(char *labels, char *description, char required, karg_type_t type, char *data) {
    // central registration point; any new argument behavior start here.
    if (labels == NULL) {
        printf("[karg] invalid labels parameter\n");
        return 0;
    }
    if (argsCount >= KARG_MAX) {
        printf("[karg] arg limit (%d) reached\n", KARG_MAX);
        return 0;
    }

    char *labelsCopy = strdup(labels);
    if (labelsCopy == NULL) {
        printf("[karg] allocation failure\n");
        return 0;
    }

    karg_t *arg = &args[argsCount];
    arg->labels = labelsCopy;
    arg->description = description;
    arg->required = required;
    arg->type = type;
    arg->data = data;
    arg->flagSet = 0;
    arg->active = 0;
    arg->children = NULL;
    arg->childrenCount = 0;
    arg->childrenCapacity = 0;
    arg->isHelp = 0;

    argsCount++;
    return (argid_t)argsCount; // 1-based id
}

argid_t karg_signParam(char *labels, char *description, char required, char *defaultData) {
    argid_t id = karg_registerRaw(labels, description, required, KARG_TYPE_PARAM, defaultData);
    if (id != 0 && rootsCount < KARG_MAX) {
        roots[rootsCount++] = id;
    }
    return id;
}

argid_t karg_signFlag(char *labels, char *description) {
    argid_t id = karg_registerRaw(labels, description, 0, KARG_TYPE_FLAG, NULL);
    if (id != 0 && rootsCount < KARG_MAX) {
        roots[rootsCount++] = id;
    }
    return id;
}

argid_t karg_signParent(char *labels, char *description) {
    argid_t id = karg_registerRaw(labels, description, 0, KARG_TYPE_PARENT, NULL);
    if (id != 0 && rootsCount < KARG_MAX) {
        roots[rootsCount++] = id;
    }
    return id;
}

static argid_t karg_addChildTo(argid_t parentId, char *labels, char *description, char required, karg_type_t type, char *data) {
    if (parentId == 0 || parentId > argsCount) {
        printf("[karg] invalid parent id\n");
        return 0;
    }
    if (ARG(parentId)->type != KARG_TYPE_PARENT) {
        printf("[karg] parent arg is not of type PARENT\n");
        return 0;
    }

    argid_t id = karg_registerRaw(labels, description, required, type, data);
    if (id == 0) {
        return 0;
    }

    karg_t *parent = ARG(parentId);
    if (parent->childrenCount >= parent->childrenCapacity) {
        int newCap = parent->childrenCapacity == 0 ? 4 : parent->childrenCapacity * 2;
        argid_t *tmp = (argid_t *)realloc(parent->children, sizeof(argid_t) * newCap);
        if (tmp == NULL) {
            printf("[karg] allocation failure\n");
            return 0;
        }
        parent->children = tmp;
        parent->childrenCapacity = newCap;
    }
    parent->children[parent->childrenCount++] = id;

    return id;
}

argid_t karg_signChildParam(argid_t parentId, char *labels, char *description, char required, char *defaultData) {
    return karg_addChildTo(parentId, labels, description, required, KARG_TYPE_PARAM, defaultData);
}

argid_t karg_signChildFlag(argid_t parentId, char *labels, char *description) {
    return karg_addChildTo(parentId, labels, description, 0, KARG_TYPE_FLAG, NULL);
}

argid_t karg_signChildParent(argid_t parentId, char *labels, char *description) {
    return karg_addChildTo(parentId, labels, description, 0, KARG_TYPE_PARENT, NULL);
}

argid_t karg_generateHelp(char *labels) {
    helpParentId = karg_registerRaw(labels, "Show this help message", 0, KARG_TYPE_PARENT, NULL);
    if (helpParentId == 0) {
        return 0;
    }
    ARG(helpParentId)->isHelp = 1;
    if (rootsCount < KARG_MAX) {
        roots[rootsCount++] = helpParentId;
    }
    return helpParentId;
}

static void karg_printHelpScope(argid_t *scope, int scopeCount, int depth) {
    for (int s = 0; s < scopeCount; s++) {
        karg_t *arg = ARG(scope[s]);

        for (int d = 0; d < depth; d++) {
            printf("  ");
        }

        char *copy = strdup(arg->labels);
        if (copy != NULL) {
            printf("  ");
            char *saveptr = NULL;
            char *token = strtok_r(copy, " ", &saveptr);
            char first = 1;
            while (token != NULL) {
                printf("%s%s", first ? "" : ", ", token);
                first = 0;
                token = strtok_r(NULL, " ", &saveptr);
            }
            free(copy);
        }

        if (arg->required) {
            printf(" (required)");
        }
        if (arg->type == KARG_TYPE_PARENT && !arg->isHelp) {
            printf(" [subcommand]");
        }
        if (arg->description) {
            printf(" - %s", arg->description);
        }
        printf("\n");

        if (arg->type == KARG_TYPE_PARENT && arg->childrenCount > 0) {
            karg_printHelpScope(arg->children, arg->childrenCount, depth + 1);
        }
    }
}

static void karg_printHelp(void) {
    printf("Usage:\n");
    karg_printHelpScope(roots, rootsCount, 0);
}

static void karg_parseScope(argid_t *scope, int scopeCount, int argc, char *argv[], int *pos) {
    // if parsing bugs appear, inspect this function first; it controls the CLI flow.
    while (*pos < argc) {
        char *token = argv[*pos];
        argid_t matchId = 0;

        for (int s = 0; s < scopeCount; s++) {
            if (karg_labelMatches(ARG(scope[s])->labels, token)) {
                matchId = scope[s];
                break;
            }
        }

        if (matchId == 0) {
            printf("[karg] unknown argument: %s\n", token);
            (*pos)++;
            continue;
        }

        karg_t *arg = ARG(matchId);

        switch (arg->type) {
            case KARG_TYPE_PARAM: {
                (*pos)++;
                if (*pos >= argc) {
                    printf("[karg] missing value for argument: %s\n", token);
                    return;
                }
                arg->data = argv[*pos];
                (*pos)++;
                break;
            }
            case KARG_TYPE_FLAG: {
                arg->flagSet = 1;
                (*pos)++;
                break;
            }
            case KARG_TYPE_PARENT: {
                arg->active = 1;
                (*pos)++;
                karg_parseScope(arg->children, arg->childrenCount, argc, argv, pos);

                if (arg->isHelp) {
                    karg_printHelp();
                    exit(0);
                }
                return; // subcommand owns the rest of argv
            }
        }
    }
}

static char karg_validateScope(argid_t *scope, int scopeCount) {
    // required-argument behavior lives here; logic consistent with the parser.
    char missing = 0;
    for (int s = 0; s < scopeCount; s++) {
        karg_t *arg = ARG(scope[s]);
        if (arg->isHelp) {
            continue;
        }

        switch (arg->type) {
            case KARG_TYPE_PARAM:
                if (arg->required && arg->data == NULL) {
                    printf("[karg] missing required argument: %s\n", arg->labels);
                    missing = 1;
                }
                break;
            case KARG_TYPE_FLAG:
                if (arg->required && !arg->flagSet) {
                    printf("[karg] missing required flag: %s\n", arg->labels);
                    missing = 1;
                }
                break;
            case KARG_TYPE_PARENT:
                if (arg->required && !arg->active) {
                    printf("[karg] missing required subcommand: %s\n", arg->labels);
                    missing = 1;
                } else if (arg->active && karg_validateScope(arg->children, arg->childrenCount)) {
                    missing = 1;
                }
                break;
        }
    }
    return missing;
}

void karg_checkArgs(int argc, char *argv[]) {
    int pos = 1;
    karg_parseScope(roots, rootsCount, argc, argv, &pos);

    if (karg_validateScope(roots, rootsCount)) {
        karg_printHelp();
        exit(1);
    }
}

char *karg_getParam(char *path) {
    argid_t id = karg_findByPath(path);
    if (id == 0 || ARG(id)->type != KARG_TYPE_PARAM) {
        return NULL;
    }
    return ARG(id)->data;
}

char karg_getFlag(char *path) {
    argid_t id = karg_findByPath(path);
    if (id == 0 || ARG(id)->type != KARG_TYPE_FLAG) {
        return 0;
    }
    return ARG(id)->flagSet;
}

char karg_isActive(char *path) {
    argid_t id = karg_findByPath(path);
    if (id == 0 || ARG(id)->type != KARG_TYPE_PARENT) {
        return 0;
    }
    return ARG(id)->active;
}