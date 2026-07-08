# karg

Lightweight C command-line argument parser.

karg is a small, dependency-free argument parser implemented in C. It
lets applications register named parameters, boolean flags, and hierarchical
parent/child command nodes, parse argv, and query values at runtime.

## Features
- Single-file C implementation (see the `karg` folder)
- Register parameters, flags, parent/child commands
- Built-in help entry generation
- Minimal API designed for embedding into small tools and utilities

## Quick start

1. Build or include the sources in your project. The headers and sources live
   under the `karg` directory.

   Example (CMake): add the `karg` directory to your target's sources or
   `add_subdirectory(karg)` and link the resulting target.

2. Include the header and register arguments in `main`:

```c
#include <stdio.h>
#include "karg/karg.h"

int main(int argc, char *argv[]) {
	/* Register a flag and a parameter with a default value */
	karg_signFlag("-v --version", "Show program version");
	karg_signParam("-o --output", "Output file path", 0, "out.txt");
	/* Add a built-in help entry */
	karg_generateHelp("-h --help");

	/* Parse and validate command-line arguments */
	karg_checkArgs(argc, argv);

	/* Query values after parsing */
	if (karg_getFlag("--version")) {
		printf("karg demo v1.0\n");
		return 0;
	}

	char *out = karg_getParam("--output");
	printf("Output: %s\n", out ? out : "(null)");
	return 0;
}
```

## API (selected)
- `argid_t karg_signParam(char *labels, char *description, char required, char *defaultData)` — register a parameter with labels and optional default
- `argid_t karg_signFlag(char *labels, char *description)` — register a boolean flag
- `argid_t karg_signParent(char *labels, char *description)` — create a parent/command node
- `argid_t karg_generateHelp(char *labels)` — register help entry labels
- `void karg_checkArgs(int argc, char *argv[])` — parse and validate
- `char *karg_getParam(char *path)` — retrieve a parameter value
- `char karg_getFlag(char *path)` — check whether a flag was set
- `char karg_isActive(char *path)` — check if a parent/command was activated

See the header for full signatures and additional helpers: [karg/karg.h](karg/karg/karg.h#L1).

## License

This project is provided under the terms of the LICENSE file.

## Contributing

Contributions, bug reports and small improvements are welcome. Open an issue
or submit a pull request with tests or minimal repro steps.

## About

I built `karg` primarily to make creating CLI C programs faster and
less repetitive. I use it for small command-line utilities, system tools,
and experiments where pulling in a larger dependency would be overkill.

Note: I enjoy working close to the metal in C, exploring command-line
usability, and sharing tiny, focused libraries that solve one problem well.
If you try `karg` and have feedback, please open an issue or send a pull
request — I read them and appreciate small examples that reproduce the issue.

