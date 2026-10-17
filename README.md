# Prabowo-Lang (prabowo)

An *Esoteric Build System* and Satirical Programming Language based on `.prabowo` files. It combines automated code compilation (similar to Makefile)

## Features & Syntax
* **Body Mode (`c{}`, `f{}`):** Modes for string literal concatenation and automatic file pattern matching (globbing).
* **Variables & Arguments (`!name`):** Handling of global variables and local function arguments.
* **Main Entry (`pidato`):** The primary execution target and entry point of the program.

## Installation & Compilation
```bash
git clone https://github.com/Rhezaagit/prabowo.git
cd prabowo
gcc ./*.c -o prabowo
```

## Usage
Run the interpreter with your script file:
```bash
./prabowo 
```
Make Sure That `exampleprabowo` renamed to `.prabowo`

## Code Example
```conf
!var = c{"keracunan " "mbg " "itu sakit perut biasa"}

bahas_mbg issues {
    c{"echo " !issues}
}

pidato {
  c{"echo " !var}
  $bahas_mbg{!var}
}
```
