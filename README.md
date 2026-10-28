# simpleshell
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.23701214.svg)](https://doi.org/10.5281/zenodo.23701214)

A C++ framework for writing line-oriented command interpreters in the style of a
Unix shell.

The `cli` library parses each line the user types: commands, arguments,
variable assignments, redirections (`<`, `>`, `>>`), pipes (`|`) and background
execution (`&`). It then calls the functions the program has registered for
each command.

## Requirements

- A C++ compiler with C++11 support
- CMake 2.6 or later
- Boost 1.44 or later
- Doxygen (optional, only to build the documentation)

## Building

```sh
cd build
cmake ..
make
```

The demo program is built as `build/bin/simpleshell` and the static library as
`build/lib/libcli.a`.

To generate the API documentation, run `make doc` (requires Doxygen).

## Usage

`src/main.cpp` is a complete example. The essentials are:

```cpp
#include <cli/shell.hpp>

bool onCommand(const std::string& command, cli::ShellArguments const& arguments)
{
    // Run the command...
    return false;   // returning true ends the interpreter
}

int main()
{
    cli::ShellInterpreter interpreter;
    interpreter.promptText("$ ");
    interpreter.onRunCommand(&onCommand);
    interpreter.loop();
    return 0;
}
```

## Layout

- `include/cli/` — public library headers
- `src/cli/` — library implementation
- `src/main.cpp` — demo program
- `doc/` — Doxygen configuration

## License

Copyright 2010-2016 Jesús Torres <jmtorres@ull.es>

Licensed under the [Apache License 2.0](LICENSE).
