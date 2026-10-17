![text](img.png)

## What is Good-Shell?

Good-Shell _(GShell)_ is a shell written in C++17 that you can use to do basic shell usage. There are 9 builtin operations that do not require external components. Those are:

* ```right``` -> returns 0
* ```left``` -> returns 1
* ```say``` -> says the user input back to standart output (usage: "say somethingsomething"), has flags:
    * **-e** enables escape characters
    * **-E** disables escape characters
    * **-n** does not print a newline at the end
* ```sad``` -> shows active directory
* ```sdir``` -> switches active directory (usage: "sdir /desired/directory/path")
* ```look``` -> looks inside a file and prints what it reads to standart output (usage: "look file.txt")
* ```set``` -> creates a new variable with a value (usage: "set VAR_NAME value")
* ```enable``` -> enables a variable so it can be used by a program, if there's no variables with that name, it creates one with no value (usage: "enable VAR_NAME")
* ```showenv``` -> prints out the current environment variables

GShell can also launch external executables, but it currently lacks better scripting abilities and pipes/redirections. (Refer to [TODO](./TODO) for planned future changes)

### External Software

This project uses external components:

* [tiny-fmt](https://github.com/monjaris/tiny-fmt) by [monjaris](https://github.com/monjaris)
* [editline](https://github.com/troglobit/editline) by [troglobit](https://github.com/troglobit)

### Build Requirements

To build with ```xmake```, you must have these software installed on your system:

* C++17 compiler (gcc, clang etc.)
* Xmake

Then you can build with ```bash build.sh```, or ```bash build_debug.sh``` for a debug build.