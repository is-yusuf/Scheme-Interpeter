# Interpreter for Scheme

This repository contains an interpreter designed to execute Scheme programs. It supports a variety of primitive operations, special forms, and data types, enabling the execution of complex Scheme scripts.

## Building the Interpreter

To build the interpreter, ensure you have the necessary build tools installed on your system. Then, navigate to the root directory of this repository and run the following command:

```bash
./just build
```

This command compiles the source code into an executable named `interpreter`.

## Running Test

After a successful build, you can verify the correctness of the interpreter by running the Knuth test:

```bash
./interpreter knuth.scm
```

This test executes a predefined Scheme script named `knuth.scm`, which is designed to test various functionalities of the interpreter.

## Features

The interpreter supports a wide range of functionalities, including but not limited to:

- **Primitive Operations:** Addition (`+`), subtraction (`-`), multiplication (`*`), division (`/`), `car`, `cdr`, and `cons`.
- **Special Forms:** `let`, `letrec`, `let*`, `lambda`, and `if`.
- **Data Types:** Integer (`int`), floating-point (`double`), and string (`str`) types, among others.

## Usage

To run a Scheme script using the interpreter, use the following command:

```bash
./interpreter <script_name>.scm
```

Replace `<script_name>` with the name of your Scheme script file.

## Acknowledgement

I build parts this project with Josh Meier for PL class.

The assignment was originally created by David Liben-Nowell and has since been updated by Dave Musicant, Jed Yang, and Laura Effinger-Dean.
