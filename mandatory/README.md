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




















