## Process-Spawn-CXX (PSCXX)

PSCXX is a simple, one-header-file big C++ wrapper written in C++17 to simplify process spawning that you can just drag and drop to your project for POSIX compliant systems.

The header contains a single function named **execute()** within the scope of the **pscxx** namespace, the arguments of this function are:

1. **"std::vector\<std::string\> execution_argv"** -> a vector of command line arguments to execute
2. **"std::string binary_path"** -> the path to the binary you want to execute
3. **"std::optional<char \*const\*> environment"** -> the environment the process will execute with, defaults to "static_cast<char \*const\*>(environ)" if no arguments for it is passed

The **execute()** function returns a **PRESULT** struct that contains:

* The exit code of the process
* The signal it exited with if it does
* The error message it returns
* The output the process returns
