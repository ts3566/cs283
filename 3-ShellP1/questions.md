1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  `fgets()` is a good choice because it reads input safely, preventing buffer overflows by limiting the number of characters read. It also handles input line-by-line, making it ideal for a command-line parser.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  Using `malloc()` allows dynamic memory allocation, which avoids stack overflow for large inputs and provides flexibility for future extensions. A fixed-size array could waste memory or be too small for some commands.


3. In `dshlib.c`, the function `build_cmd_list()` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  Trimming leading and trailing spaces ensures commands are correctly parsed and executed. Without trimming, extra spaces could cause issues like incorrect command recognition, failed executions, or misinterpretation of arguments.

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  We could implement the following standard redirections for our custom shell:
    >
    > First, redirecting STDOUT to a file (ls > output.txt), which sends command output to a file. Challenges could be handling file permissions, managing file modes, and ensuring proper buffering.
    >
    > Second, redirecting STDERR to a file (ls /nonexistent 2> error.txt), which captures errors separately. Challenges include managing file ensuring errors don’t interfere with STDOUT, and handling concurrent output.
    >
    > Third, redirecting STDOUT and STDERR to the same file (ls /nonexistent > output.txt 2>&1), which combines both streams. Challenges include correct redirection order, and proper file descriptor duplication.

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  Redirection changes where the input or output of a command comes from or goes to, such as redirecting output to a file (command > file.txt). Piping, on the other hand, allows the output of one command to be passed as input to another command (command1 | command2).

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  By separating regular output (STDOUT) from error messages (STDERR), users can easily distinguish between successful results and errors. This also allows for more flexible handling of data, such as redirecting errors to a log file while still displaying regular output on the terminal.

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  In a custom shell, when a command fails and produces both STDOUT and STDERR, it’s important to handle the separation and merging of outputs based on the user's intent. By default, the shell should keep these outputs separate for clarity. However, if the user requests merged output, the shell can provide an option to combine STDOUT and STDERR. This can be done using the `2>&1` syntax, which redirects STDERR to the same location as STDOUT.
