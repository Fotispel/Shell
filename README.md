# Shell

A Unix‑like shell implemented in C for the HY345 Operating Systems course.
This project demonstrates core OS concepts such as process creation, command parsing, file redirection, and piping.

---

## Features

* Execute external commands (`ls`, `cat`, `grep`, etc.)
* Support for **pipes** (`|`)
* Support for **I/O redirection** (`>`, `<`)
* Basic **environment variable expansion** (`$HOME`, `$USER`, ...)
* Support for **arguments** and **multiple‑word commands**
* Clear and modular code structure
* Makefile included

---

## Project Structure

```
hy345sh.c        // Main implementation of the shell
Makefile         // Build instructions
README.md        // Project documentation
```

---

## Building the Project

Make sure you have GCC installed, then simply run:

```bash
make
```

This will compile the shell and produce an executable named `hy345sh`.

To clean up object files and the executable:

```bash
make clean
```

---

## Running the Shell

After building, run:

```bash
./hy345sh
```

You will be presented with a minimal shell prompt where you can execute commands.

---

## Examples

**Run a command:**

```
shell > ls -l
```

**Pipe output:**

```
shell > ls | grep .c
```

**Redirect output to a file:**

```
shell > cat input.txt > out.txt
```

---

## Course Context

This project was created as part of the **HY345 – Operating Systems** course at the University of Crete.
It focuses on understanding system calls such as:

* `fork()`
* `execvp()`
* `waitpid()`
* `open()` / `dup2()`
