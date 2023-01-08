# GETTING STARTED

Pipex reproduces the behaviour of the shell pipe | command in c it launches as
`.pipex infile cmd1 cmd2 outfile` and behaves as this line does in the shell `<infile cmd1 | cmd2 > outfile`.

in this project you'll encounter alot of new concepts:


* ***processes and inter-communication between processes***


but we wond start whit that we'll just start with basic functions and systeme calls and the logic behind the project :

if you want to read the [subject](https://github.com/RIDWANE-EL-FILALI/PIPEX/blob/master/en.subject.pdf)

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

To reproduce this behaviour, we could duplicate the write end of the pipe over the standard output in the first child, and the read end over the standard input of the second child. we've previously learnt about the **dup2** function that would allow us to do this in the article about file descriptors

## CREATING A PIPELINE LIKE A SHELL
Of course, a shell can string multiple commands together with the “`|`” operator. For example, we can do commands like `man bash | head -n 50 | grep shell | grep bash | wc -l`. This is called a pipeline.

If we tried to use a single pipe for all of the child processes’ inputs and outputs in our endeavor to replicate this kind of pipeline, we’d quickly encounter big issues. Since the child processes are executed simultaneously, they will start fighting to read from and write to a single pipe. And one will inevitably end up waiting for input that will never arrive.

To build a pipeline, then, we need to create a pipe (a pair of file descriptors) for each child process, minus 1. That way, the first child can write on its own pipe, the second can read from the first one’s and write to its own, and so on.

![image](https://github.com/RIDWANE-EL-FILALI/PIPEX/blob/master/img/pipeline_diagram_en.drawio.png)

## FILE DESCRIPTORS 
in unix type systems a **file descriptor** is a small positive integer used as reference to an open file in a process. A process as we've seen in a previous article about processes ia a currently running program

However from the operating system point of view a file is not a text file as we might think of it as a user a file can also be a directory or a i/o resource such as keyboard a screen a pipe, or a network socket
by default, each process systematically inherits three open file descriptors

| File descriptor| Name|<Unistd.h>|<stdio.h>|
|--------------|------|-----------|----------|
|0|Standard Input| STDIN_FILENO| stdin|
|1|Standard Output| STDOUT_FILENO| stdout|
|2|Standard Error| STDERR_FILENO| stderr|

But why use file descriptors as identifiers? An integer is much simpler to process for a computer than a long path string to a file. What’s more, the reference to a file must contain much more than its location: it must also include its permissions, access mode, size, etc… And wouldn’t it be wasteful to keep several entire references to the same file if several processes opened it?

So how does the operating system maintain all of the information about every open file?

## THE SYSTEM'S REPRESENTATION OF OPEN FILES

To represent open files, the system uses three data structures:

* A **table of file descriptors** per process. Each process has its own table containing a series of indexes, each one referring to an entry in the open file table.
* An **open file table** shared between all processes. each entry in this table contains, among other things, the access mode, an offset describing the current location within the file, and a pointer to the corresponding entry in the inode table. this table also kepps count of the number references there are to this file in all of the file descriptor tables of all processes. when a process closes the file, this reference count is decremented and if it gets to 0, the entry is deleted from the table
* An **inode (index table) table** which is also shared between all processes. each entry in the inode table describes the file in detail: the path to its location on the disk, its size, its permissions, etc
![image](https://github.com/RIDWANE-EL-FILALI/PIPEX/blob/master/img/file_descriptors_en.drawio.png)

This diagram shows the organization of references to files in use by three processes.

Two processes can of course have the same file open: process A can access file B throught its descriptor 4, and the same is true for process B and its descriptor 3. this fact can help facilitate inter-process communication

A process can also have two or more references to the same file, as with process C. this can happen when we open the same file twice. later we will see why such a thing could come in handy.

# OPENING OR CREATING A FILE IN C
To manipulate a file in c, we must first inform the operating system of our intensions with the **open** funtion of the <fcntl.h> library. this system call allow us to open an exesting file, or create the file if doesn't already exist. we must at least specify the path towards the file we'd like to open, as well as the way in wich we want to access it
```
int open(const char *pathname, int flags); 
int open(const char *pathname, int flags, mode_t mode);
```
So there ar two versions of thi open system call their parameters are:

* **Pathname** : the path towards th file in the form of a string of characters,
* **Flags** : an integer representing the flags indicating the access mode which we will take a closer look at below
* **Mode** : an integer representing the permissions to give to the file upon creation this is an "optional" parameter that will be ignored if we don't ask to create the file it it doesn't already exist

## The open access mode :
The flags parameter in the open system call allows us to choose the file's access mode . the <fcntl.h> library contains a large selection of symbolic constants among theme the most common one are:

-------------------------------------
|**Symbolic Constant**|**Description**|
|------------|-------------------|
|O_RDONLY| Read only|
|O_WRONLY|Write only|
|O_RDWR|Read and write|
|O_TRUNC|truncated mode. if the file exists and the access mode allows writing to it ( O_WRONLY OU O_RDWR), truncates its contents to 0 size when it is opened with the effect of overwriting the file|
|O_APPEND|Append mode. the file offset is set to end of the file at each write, with the effect of adding text to end of the file instead of overwriting it.|
|O_CREAT|Creates the file if it does not already exist. its permissions must then be specific in open's mode parameter

for example we can open a file in read only mode this way:
```
open("paths/to/file", O_RDONLY);
```
but we can also combine flags with the bitwise | (or) operator for example if we wanted to open a file in truncated write mode, we could do:
```
open("paths/to/file", O_WRONLY | O_TRUNC);
```
let's note that indicating an access mode with O_RDONLY , O_WRONLY or O_RDWR is mandatory we could not for example simply specify O_CREAT without any other indication of an access mode

## Creating a file with open's O_CREAT option

as we've previously seen, open allows us to create a file if the specified file does not already exist for that we need to indicate the O_CREAT symbolic constant in its flags parameter in this case we must describe in the following optional parameter mode which permissions to give the new file

|**Symbolic Constant**            | **Decimal**                                       |**Description**|
|-------------------------|-------------------------|-----------------------------|
|S_IRWXU|700|The owner(user)has read, write and execute rights|
|S_IRUSR|400|The owner has read rights|
|S_IWUSR|200|The owner has write rights|
|S_IXUSR|100|The owner has execute rights|
|S_IRWXG|070|The group has read write and execute rights|
|S_IRGRP|040|The group has write rights|
|S_IWGRP|010|The group has execute rights|
|S_IRWXO|007|The users have read write and execute rights|
|S_IROTH|004|Others have read rights|
|S_IWOTH|002|Others have write rights|
|S_IXOTH|001|Others have execute right|

of course we can also combine these symbolic constants with the same bitwise OR operator as before for example to create a file in append write only mode with read and write permissions for the owner but only the read permission for the group
```
open("path/to/file", O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP);
```
an alternative to shorten this permissions specification is to directly use their added numeric values, preceded by a 0 . The following is exactly equivalent to the previous example:
```
open("path/to/file", O_WRONLY | O_APPEND | O_CREAT, 0640);
```

# CLOSING A FILE DESCRIPTOR
When we are done manipulating a file we must of course de-reference its file descriptor with the **close** function of the <fcntl.h> library its prototype could not be simpler
```
int close(int fd);
```
we supply it with a file descriptor and the system de-references it. and if no other process has that file opened delete it from its open file and i-node table. Upon success the close function returns 0, but on failure it returns -1 and sets **errno** to indicate the error
However, the close function only closes the file descriptor, it does not delete the file itself that is unlink's prerogative

# Deleting a file in c with unlink
If we wish to completely delete a file on the hard drive with a c program, we can use the **unlink** system call from <unistd.h> library its prototype is:
```
int unlink(const char *pathname);
```
all we have to indicate here is the path towards the file and it will be deleted once all processes using it have closed their descriptors referencing it. this system call returns 0 on success or -1 on failure.

Of course, closing and deleting a file right after opening it serves no purpose: we probably want to do something with that opened file. why not write to it

# Writing to a file Descriptor in c
once a file descriptor is opened with an access mode permitting writing we will be able to write to the file it reference thanks to the write system call from the <unistd.h> library its prototype is:
```
ssize_t write(int fd, const void *buf, size_t count);
```
And its parameters are follows:
* **fd:** the file descriptor to write to*
* **buf:** a pointer to a memory zone to write to the file, typically containing a string of characters
* **count:** The number of bytes to write typically the length of the string of characters specified in the previous parameter.

on success the write function returns the number of bytes it has written, however upon failure it returns -1 and sets errno to indicate the error it encountered

# Reading from a file Descriptor in c
its parameters are as follows:

* **FD:** the file descriptor to read from
* **BUF:**  a pointer towards a memory area where we can temporarily store the read characters
* **COUNT:** a size in bytes to read, in other words, the number of characters to read. This size will often correlate with the size of the memory area indicated in the previous parameter.
Then the read functions returns the number of characters that it has read or -1 in case of errno when the read function reaches the end of the file it will naturally returns 0
do the read function stops reading when it reaches  the number of characters we indicated or end of file(EOF)
The read function seems to have an integrated bookmark! at each call read resumes where it left off last time. in truth the read function does not remember its last position  in the file itself it only increments the file descriptor offset

# A file Descriptor's offset
As we may have notices in the diagram at the beginning of this article, the reference in the common table of open files contain an offset. the offset represents the current number of bytes from the beginning to the file, which gives us the current position in the file this is what the read function increments at the end of it's execution

So when we open a file, the offset is typically 0, which places us at the beginning of the file when se read let's say 12 characters, the offset is set to 12. the next time we access the file descriptor to read or even write we will start from the current offset from the beginning of the file in this case the 13<sup>th</sup> character.

Both the read and write system calls are affected by the file descriptor's offset

# Resetting the file Descriptor offset with a new file Descriptor

The simplest solution might be to open the same fie again with the open function system call. this creates a new entry in the system's open file table, with an offset of 0 by default.

# Adjusting the file Descriptor offset with Lseek
There is another option to set a file descriptor's offset to the position we want . Its the **lseek** function of the <unistd.h> library it allows us a much finer control over our position in the file. its prototype is:
```
off_t lseek(int fd, off_t offset, int whence);
```
let's take a closer look at its parameters:
* fd: the file descriptor for which to change the offset
* offset: the number of bytes to move the offset
* whence: the position from which to apply the offset possible options are:*
	* SEEK_SET: Apply the offset relative to the beginning of the file
	* SEKK_CUR: Apply the offset relative to the current position in the fie
	* SEEK_END: Apply the offset relative to the end of the file
We must be careful with lseek since it allows us to set our offset past the end of the file!if we write past the end of the file, the file's size will not change and it will create "holes" zones filled with \\0 in the middle of the file.

if lseek succeeds it returns the new offset meaning the total number of bytes from the beginning of the file. on failure it returns -1 and sets errno to indicate the error code.

# Duplicating file Descriptors with dup/dup2
it can sometimes be useful to duplicate a file descriptor in order to save it as a backup or replace another one. this is the case for example, in the context of a standard input or output redirection towards a file

The **dup** and **dup2** system calls from the <unistd.h> library enable us to duplicate a file descriptor here are their prototypes:
```
int dup(int oldfd); 
int dup2(int oldfd, int newfd);
```
Both take the file descriptor we want to duplicate (oldfd) as a  parameter and returns the new file descriptor , or -1 in case of error the difference between the two is that dup automatically chooses the smallest unused number for the new file descriptor, whereas with dup2 we can specify which number we want (newfd)

we have to keep in mind that dup2 is going to try to close the newfd if it is in use before transforming it into a copy of oldfd. however if oldfd is not a valid file descriptor the call will fail and newfd will not be closed if oldfd and newfd are identical and valid, dup2 will just return the newfd without doing anything further

# The interchangeability of Duplicated Descriptors
After a successful call to dup2 or dup, the old and the new file descriptors are interchangeable they refer to exact same open file and share its offset and other attributes. For example if we read the first few characters of a file with read function with one of the descriptors, The offset will be changed for both descriptors not only the one we read with

Yet we previously saw that if we opened the same file twice the two descriptors did not share their offsets in this way. So why doe it work differently for duplicated file descriptors, The reason for this is best illustrated with a diagram:

![image](https://github.com/RIDWANE-EL-FILALI/PIPEX/blob/master/img/file_descriptors_open_vs_dup_en.drawio.png)

a file descriptor opened with open has its own entry in the system's open files table, with its own offset however a duplicated file descriptor shares the open files table entry with its clone, which means they also share an offset

# **OPEN()** 
## int open(const char \*path, int flags, ..);
	The file name specified by path is opened for reading and/or writing as specified by the argument oflags, the file descriptor is returned to the calling process


# **WRITE()** 
## ssize_t write(int fields, const void \*buf, size_t nbytes);
	write() attempts to write nbyte of data to the object referenced by the descriptor fildes from the buffer pointed to by buf.

# **Close()**
## int close(int fields)
delete a descriptor
The close() call delete a descriptor from the per-process object reference table.

# **READ()**
## ssize_t  **read**(int fildes, void \*buf, size_t nbyte);
**read**() attempts to read nbyte bytes of data from the object referenced by the descriptor fildes into the buffer pointed to by buf

# **PERROR()**
## void perror(const char \*str);

The C library function void **perror**(const char \*str) prints a descriptive error message to stderr first the string str is printed followed by  colon then a space
this function does not return any value.

# **STRERROR()**
## char \*strerror(int errnum)
Searches an internal array for the error number errnum and returns a pointer to an error message string. The error strings produced by strerror depend on the developing platform and compiler
the strerror function prints the phrase that belongs to the exact number
errno in a global variable that is in the library <errno.h>
# **ACCESS():**
## int access(const char \*pathname, int mode);
in linux, access command is used to check whether the calling program has access to specified file. it can be used to check whether a file exists or not. the check is done using the calling process's real UID and GID
here, the first argument takes the path to the directory/file and second argument takes R_OK, W_OK, X_OK, F_OK
* **F_OK FLAG**: Used to check for existence of file
* **R_OK FLAG**: Used to check for read permission bit
* **W_OK FLAG**: Used to check for write permission bit
* **X_OK FLAG**: Used to check for execute permission bit
note : if access() cannot access the file, it will return -1 or else it will be 0

# **DUP():**
## int dup(int oldfd);
The dup() system call creates a copy of a file descriptor.
* it uses the lowest-numbered unused descriptor for the new descriptor.
* if the copy is successfully created, then the original and copy file descriptors may be used interchangeably
* they both refer to the same open file descriptors and thus share file offset and file status flags

# **DUP2()**
## int dup2(int oldfd, int newfd);
The dup2() system call is similar to dup() but the basic difference between them is that instead of using the lowest-numbered unused file descriptor it uses the descriptor number specified by the user
* include the header file unistd.h for using the dup2() system call
* if the descriptor newfd was previously open, it is silently closed before being used
* if oldfd is not a valid file descriptor, then the call fails, and newfd is not closed
* if oldfd is a valid file descriptor, and newfd has been the same value as oldfd then dup2() does nothing and returns newfd

# **EXECVE():**
## int execve(const char \*filename, char const \*\*av, char const \*\*envp );
The execve() system call loads a new program into a process's memory
the existing process is discarded
the newly create process gets all new stack, data, & heap
since it replaces the process's memory, the return value is always -1
**Parameters:**
* pathname : The pathname of the command to execute
* argv : The arguments to pass to the new program
* envp : The environment list
Note that agrv and envp are arrays of character pointers.

```
#include <stdio.h>
#include <unistd.h>

int main(void)
{
	char cmd[] = "/usr/bin/ls";
	char *argvec[] = {"ls", "-l", NULL};
	char *envvec[] = {NULL};
	printf("Start of execve call %s\n", cmd[0]);
	printf("===================================\n")
	if (execve(cmd, argvec, envvec) == -1)
		perror("could not execute");
	printf("oops, something went wrong!");
	

	return (0);
}
```
 
# **EXIT():**
## void exit(int code)
The exit function is used to break out of a loop this function causes an immediate termination of the program done by the operation system
the value of the code is returned to the calling process which is done by an operation system generally zero is used as return code to indicate normal program termination.

# **fork():**
fork() is a system call used to create a new process this newly created process is known as a child process with a pid of 0 and the process that created it is called a parent process the fork returns the pid of the newly created process
and -1 in case of error and a positive value returned to the calling process

# **pipe():**
## int pipe(int fd[2]);
pipe() is a unix,linux system call that is used for inter-process communication
fd[0] read end of the pipe;
fd[1] write end of the pipe;

return 0 on success and -1 on error
# **UNLINK():**
you can delete a file with unlink or remove

# **WAIT():**
## pid_t wait(int \*start_loc)
A call to wait() blocks the calling process until one of its child processes exits or a signal is received after child process terminates parent continues its execution after wait system call instruction
child process may terminate due to any of these:
* it calls exit()
* it returns (an int) from main
* it receives a signal (from the os or another process) whose default action is to terminate

![](https://media.geeksforgeeks.org/wp-content/uploads/Wait_system_call_in_c.jpg)

if any process has more than one child processes, then after calling wait(), parent process has to be in wait state if no child terminates.
if only one child process is terminated, then  wait() returns process ID of the terminated child process.
if more than one child processes are terminated then wait() reap
the wait function returns the process id of the process that it was waiting for

# **WAITPID():**
## pid_t waitpid(pid_t pid, int \*status_ptr, int options);
wait for a specific child process to end



# FROM THE MAN
```
int execve(const char *path, char *const argv[], char *envp[]);
* path: the path to our command  
        type `which ls` and `which wc` in your terminal  
        you'll see the exact path to the commands' binaries
* argv[]: the args the command needs, for ex. `ls -la`  
          you can use your ft_split to obtain a char **  
          like this { "ls", "-la", NULL }  
          it must be null terminated
* envp: the environmental variable  
        you can simply retrieve it in your main (see below)  
        and pass it onto execve, no need to do anything here  
        in envp you'll see a line PATH which contains all possible  
        paths to the commands' binaries
        
int main(int ac, char **ag, char **envp)  
{  
     int f1;  
     int f2;     f1 = open(ag[1], O_RDONLY);  
     f2 = open(ag[4], O_CREAT | O_RDWR | O_TRUNC, 0644);  
     if (f1 < 0 || f2 < 0)  
          return (-1);  
     pipex(f1, f2, ag, envp);  
     return (0);  
}
```
To see what is inside `envp`, type `env` in your terminal.  
You’ll see a line `PATH` , those are all the possible paths to the command binaries.  
You’ll need to split: you can use `:` as a delimiter, and retrieve them (small note on this in section 4).  
Your execve function will have to try every possible path to the cmd until it finds the good one.  
To see the path to the command `ls`, for ex., you can type `which ls` in your terminal. If the command does not exist, execve will do nothing and return -1; else, it will execute the cmd, delete all ongoing processes (including variables, so no need to worry about free) and exit (more on this, see section 3 below). In pseudo code,
```
// parsing (somewhere in your code)
char *PATH_from_envp;  
char **mypaths;  
char **mycmdargs;

// retrieve the line PATH from envp  
PATH_from_envp = ft_substr(envp ....);  
mypaths = ft_split(PATH_from_envp, ":"); // see section 4 for a  
                                            small note[0]  
mycmdargs = ft_split(ag[2], " ");
// in your child or parent processint  i;  
char *cmd;

i = -1;  
while (mypaths[++i])  
{  
    cmd = ft_join(mypaths[i], ag[2]); // protect your ft_join  
    execve(cmd, mycmdargs, envp); // if execve succeeds, it exits  
    // perror("Error"); <- add perror to debug  
    free(cmd) // if execve fails, we free and we try a new path  
}  
return (EXIT_FAILURE);
```

## 2 — How to do pipex with two child processes

We can divide the pipex workload on different children, while the parent waits for the job to be done and supervise the children’s _status_.

We will thus have to fork twice, and assign child1 to execute cmd1, and child2 to execute cmd2. The parent will wait at the end. In pseudo code,

```
void    pipex(int f1, int f2, char *cmd1, char *cmd 2)  
{  
    int   end[2];  
    int   status;  
    pid_t child1;  
    pid_t child2; 
       
    pipe(end);  
    child1 = fork();  
    if (child1 < 0)  
         return (perror("Fork: "));  
    if (child1 == 0)  
        child_one(f1, cmd1);  
    child2 = fork();  
    if (child2 < 0)  
         return (perror("Fork: "));  
    if (child2 == 0)  
        child_two(f2, cmd2);  
    close(end[0]);         // this is the parent  
    close(end[1]);         // doing nothing  
    waitpid(child1, &status, 0);  // supervising the children  
    waitpid(child2, &status, 0);  // while they finish their tasks  
}
```

## 3 — The `access()` function

If you run `< infile pikachu | ls > oufile` in the shell, you’ll get `-bash: pikachu: command not found`.  
If you now run `./pipex infile pikachu ls outfile`, your program will do nothing and exit without error messages. `execve()` will execute nothing if the command is not found. You need to check if it exists before its execution.  
To do this, check the `access()` function. With the proper mode, you can use it to verify whether the command exists and is _executable_.

## 4—Frequently encountered problems
[0] As for splitting your envp path, print out the result of your split and have a look. Add a `/` at the end for the path to work correctly.

[1] If you launch your program and it gets stuck without executing anything, most probably one end of the pipe has not been closed correctly. Until one end is open, the other will be waiting for input and its process will not finish.

[2] Use `perror("Error")` to debug, especially right after `fork()` or `execve()`, to see what is going on in the pipe. Inside the pipe, everything done goes to one of its ends. printf for ex. won’t print to the terminal or it will print to your outfile (because we swapped the stdout); `perror("Error")` will work because it prints to stderr.

[3] Handle file rights when you `open()` them. Return error if the file cannot be opened, read or written. Check how the shell treats infile and outfile when they do not exist, are not readable, writable etc. (`chmod` is your best friend).













