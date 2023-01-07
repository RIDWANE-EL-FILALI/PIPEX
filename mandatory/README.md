# GETTING STARTED

Pipex reproduces the behaviour of the shell pipe | command in c it launches as
`.pipex infile cmd1 cmd2 outfile` and behaves as this line does in the shell `<infile cmd1 | cmd2 > outfile`.

in this project you'll encounter alot of new concepts:


* ***processes and inter-communication between processes***


but we wond start whit that we'll just start with basic functions and systeme calls and the logic behind the project :

## LOGIC 
```
./pipex infile cmd1 cmd2 outfile

                                                      The main process
                                                            |
                                                            |
                                                            |
                                                          pipe()
                                                            |
                                                            |
                                                          fork()------------------------first child------
                                                            |                                            |
                                                            |                                           cmd1
                                                            |                                            |
                        --------  second child. ---------  fork()                                        dup2()
                       |                                    |                                            |
                       |                                    |                                          close(end[0])
                       |                                    |                                              |
                      cmd2                                  |                                         execve(cmd1)
                       |                                    |
                      dup2()                                |
                       |                                    | 
                   close(end[1])                            |          
                       |                                    | 
                   execve(cmd2)                             |
                                                            |
                                                            |
                                                            |
                                                            |
                                                    waitpid(first child)     
                                                            |
                                                    waitpid(second child)     
```

The point is we read from **infile**, execute cmd1 with **infile** as input, send the output as an input for cmd2, which will write to **outfile.**

```
every command needs an stdin and out 

          infile                                                                            outfile
       as stdin for cmd1                                                               as stdout for cmd2
            |                             |------------------------------|                    |
            |                             |                              |                    |
            |                             |             PIPE             |                    |
            ↓                             |                              |                    |
           cmd1---------------------->.end[1]           <-->          end[0].--------------> cmd2
                                          |                              |
            cmd1                          |                              |                   end[0]
  output is written to end[1]             |                              |         reads end[1] and sends cmd1
  the write end of the pipe               |------------------------------|         output to cmd2 (end[0] becomes
  (end[1] becomes cmd1 stdout)                                                             cmd2 stdin)

```

## PROCESS CONTROLLING IN C
### FORK()
When using the funtion fork() a child process gets born the fork funtion returns an id
```
int id = fork();
```
the id of the child process is always 0, you can calculate the number of child processes using the formula 2<sup>n</sup>
keep in mind that when a process is duplicated the memory and open fd are duplicated too.
it means that even the variables are duplicated but that does not mean that if one is changed the second will.
The order of the processes is unpridectable so to solve this probleme that when the **wait()** comes in
### WAIT()
makes a process wait for the duplicated process(the child)
### GETPID()
get the process id for of current process
### GETPPID()
get the parent process id of the current process

## WHAT IS A PIPE ?

A pipe is a section of shared memory meant to facilitate the communication between processes.
it is a undirectional channel that has a read and write end.
so a process can write to the write end of the pipe the data stored in memory buffer until it is read by another process from the pipes read end

![image](https://github.com/RIDWANE-EL-FILALI/PIPEX/blob/master/img/pipe_diagram_en.drawio.png)

A pipe is a sort of file, stored outside of the system that has no name or any other particular attribute. but we can handle it like a file thanks to its two file descriptors.
in a nutshell, a file descriptor is an integer pointing to an open file in the open file table in a certain process
let's keep in mind that there is a limit to a pipe's size which varies depending on the operating system when this limit is reached, a process will no longer be able to write it until another reads enough data from it

## CREATING A PIPE
We can create a pipe with the aptly-named pipe system call. here is its prototype in the <unistd.h>
library
```
int pipe(int piperf[2]);
```
as its only parameter, pipe takes an array of the two integers where the two file descriptors should be stored. of course these file descriptors represent the pipe's two ends

* pipefd[0] : the **read** end
* pipefd[1] : the **write** end

the pipe system call will open the pipe's file descriptor and then fill them into the provided table.
on sucess pipe returns 0, on error it returns -1 and describes the encountered error in errno, without filling the provided table

in order to establish inter-process communication between a parent and a child process, we will first have to create a pipe then fork() so the child will have a duplicate of the pipe's descriptors, since a child process is a clone this way the child will be able to read from pipefd[0] information written by the parent in pipefd[1] and vice versa.
of course we can allow two child processes to cummunicate using the same methode

## READING AND WRITING IN A PIPE
A pipe's file descriptors aren't very different from other regular descriptors, to input or retrieve data from one, we can use the read and write system calls from the <unistd.h>
library

* if a process attempts to read from an empty pipe. read will remain blocked until data is written to it
* inversely if a process tries to write to a full pipe(one that reached its size limit) write will remain blocked until enough data has been read to allow the write operation to complete.

## CLOSING A PIPE
the read and write end of a pipe can be blocked with the close system call
just like any other file descriptors however there are a few aspects to beware of when closing its descriptors
when all of the file descriptors referring to the write end of the pipe are closed, a process attempting to read from the read will receie EOF(end of file) and the read system call will return 0
the same goes for the write 

to ensure that the process correctly receive the termination indicators (EOF SIGPIPE/EPIPE) it is essential to close all unused file descriptors otherwise we risk processes getting stuck in a suspended state

## REPRODUCING THE SHELL'S PIPE "|" OPERATOR
The first thing you might notice is that when we do `cat test.txt | wc -l`, the contents of the file don;t appear at all.So what is this "|" operator doing here exactly ?

The shell creates a pipe and two child processes, one for the `cat` command and one for `wc`. Then, it redirects `cat`‘s standard output towards `wc`‘s standard input. Therefore, the `cat` command does not write its output in the standard output (our terminal), but rather in the pipe. Then, the `wc` command will go looking for the data in that pipe rather than the standard input. Here’s a little diagram to visualize the idea:

![image](https://github.com/RIDWANE-EL-FILALI/PIPEX/blob/master/img/shell_pipe_en.drawio.png)

To reproduce this behaviour, we could duplicate the write end of the pipe over the standard output in the 
















