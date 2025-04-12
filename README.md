## Getting Started
## Prerequisites
    Before building the shell, make sure you have the readline development library installed:
        bash sudo apt-get install libreadline-dev
## Building the Project
* Compile the source code using GCC:
    bash gcc main.c shell_functions.c -o shell
* Running the Shell
Execute the compiled binary:
    ./shell

## Usage Notes ---- אם להשאיר את זה בכלל 
* When using the grep command, if you use in string enclose the search pattern in quotes:
    grep "pattern1 pattern2" file.txt
    grep pattern file.txt - work 
    grep "pattern" file.txt - work

* To finish input when using cat to create a file, press Ctrl+D
* To exit the shell, type exit or press Ctrl+C

## Project Structure
* shell.h: Contains function declarations and structure definitions
* shell_functions.c: Implementation of shell functions
* main.c: Main program logic

