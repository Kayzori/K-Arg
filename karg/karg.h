#ifndef KARGS_H
#define KARGS_H

/*
 * Public interface for a lightweight command-line argument parser.
 *
 * This module allows applications to register parameters, flags, and
 * parent/child command structures, parse them from argv, and query the
 * resolved values in a simple, reusable manner.
 */

typedef unsigned char argid_t;

/* Register a named parameter argument that may accept a value. */
argid_t karg_signParam(char *labels, char *description, char required, char *defaultData);
/* Register a named boolean-style flag argument. */
argid_t karg_signFlag(char *labels, char *description);
/* Register a parent/command node that can contain child arguments. */
argid_t karg_signParent(char *labels, char *description);

/* Register a child parameter under an existing parent node. */
argid_t karg_signChildParam(argid_t parentId, char *labels, char *description, char required, char *defaultData);
/* Register a child flag under an existing parent node. */
argid_t karg_signChildFlag(argid_t parentId, char *labels, char *description);
/* Register a child parent node under an existing parent node. */
argid_t karg_signChildParent(argid_t parentId, char *labels, char *description);

/* Register a built-in help entry that can display usage information. */
argid_t karg_generateHelp(char *labels);

/* Parse the supplied command-line arguments and validate required input. */
void karg_checkArgs(int argc, char *argv[]);

/* Retrieve the value associated with a parameter by its path. */
char *karg_getParam(char *path);
/* Retrieve the state of a flag by its path. */
char karg_getFlag(char *path);
/* Determine whether a parent/subcommand node was activated. */
char karg_isActive(char *path);

#endif // KARGS_H