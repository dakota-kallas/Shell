# Simple UNIX Shell

## Author(s):

- <a href="https://github.com/dakota-kallas">Dakota Kallas</a>
- <a href="https://github.com/jack-parkerr">Jack Parker</a>

## Description:

The objective of this project is to build a simple Unix shell, which is a command-line interpreter that allows users to interact with a computer's operating system. The shell should provide a prompt that accepts commands from the user and executes them using child processes. The shell should also support both interactive and batch modes.

The shell's interactive mode should continuously prompt the user for input and execute commands in response. The shell should also provide some basic features such as command history. The user should be able to execute any command that the operating system supports, including running executables and modifying files.

In batch mode, the shell should accept a file name as input and execute a list of commands from that file. The shell should be able to handle errors, such as when a command does not exist or when a file cannot be executed.

This project was written for a partner project in CS441 at the University of Wisconsin-La Crosse during the Spring 2023 semester.

## How to build the software

The included Makefile is used to build this software. It can be used as follows:

- To build the software, run `make` from the command line.
- To build the software and run the given tests, run `make check` from the command line.
- To clean the directory, run `make clean` from the command line.

## How to use the software

This software supports two different mode of use as described below.

### Interactive Mode

Interactive mode allows users to interact with the Unix shell through a prompt that displays as `mysh$`. Users can enter commands after the prompt, and the shell will interpret and execute them. The shell will execute the jobs contained in the command sequentially, unless the user specifies a job separator. Once all the jobs are completed, the shell will return the prompt for the user to enter another command.

During the interactive mode, the shell should support basic features such as background jobs and command history. Command history should provide a list of previously entered commands, and the user can access them by scrolling through the history.

### Batch Mode

Batch mode allows the shell to execute a list of commands contained in one or more batch files, without displaying a prompt. The shell is started by specifying one or more batch files on the command line, which contains a list of commands that should be executed. In the case of multiple batch files, the shell should execute them sequentially in the order they are presented on the command line.

If there are background jobs in multiple batch files, the shell should run them concurrently. The shell should wait for all background processes to finish before exiting the shell once all batch files have been processed. The job totals displayed when the shell is exiting will represent all of the jobs started by all of the batch files.

Most batch files will not contain the `exit` command and will terminate by reaching the end-of-file. If the shell encounters the `exit` command in a batch file, it must skip the rest of that batch file and also skip the remaining batch files that have not yet been processed. The `exit` command will start to shut down the shell, waiting for any outstanding background processes to complete before displaying the shell job statistics and terminating.

## How the software was tested

This repository is set up with an automatic set of tests that can be run and checked against for a valid output with the `make check` command. Those tests can be found in the `.\given-tests\` directory, along with their expected outputs. There is another set of tests that can be found in the `.\tests\` directory which help test behavior of the program given behavior that is either unusual or touches a specific requirement of the tool. Details on each test can be found in the `README.md` in the `.\tests\` directory.
