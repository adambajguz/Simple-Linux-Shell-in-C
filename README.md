# Simple-Linux-Shell-in-C
Simple Linux shell written in C with background execution, piping and output redirection.

Features:
 - Two shell operating modes: interactive and batch (running scripts);
 - Starting programs using the PATH variable;
 - Ability to run the program in the background using & as the last command word;
 - The shell finishes its work after receiving the end of file (EOF or Ctrl + D);
 - Ability to redirect the standard output of the command with >>;
 - Possibility to create pipes of any length using the | character;
 - Supports commands with up to 256 arguments;
 - Supports commands which length is either up to BUF_SIZE macro value in interactive mode or up to theoretically infinty in batch mode;
 - Command history - the shell stores the exact content of the last 20 commands in the file history.txt in the user's home directory;
 - SIGQUIT signal sending displays the command history on the standard output.
