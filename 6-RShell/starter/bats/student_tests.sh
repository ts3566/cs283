#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Exit command works properly" {
    run "./dsh" <<EOF
exit
EOF
    
    [ "$status" -eq 0 ]
    
    [[ "$output" == *"dsh4> "* ]]
    [[ "$output" == *"cmd loop returned 0"* ]]
}

@test "Empty command works" {
    run "./dsh" <<EOF

exit
EOF
    
    stripped_output=$(echo "$output" | tr -d '\r')

    [[ "$stripped_output" == *"warning: no commands provided"* ]]

    [ "$status" -eq 0 ]
}

@test "cd with valid directory changes directory" {
    mkdir -p /tmp/dsh-test-dir
    
    run "./dsh" <<EOF
cd /tmp/dsh-test-dir
pwd
exit
EOF

    [[ "$output" == *"/tmp/dsh-test-dir"* ]]

    rmdir /tmp/dsh-test-dir
}

@test "cd with no arguments doesn't change directory" {
    current_dir=$(pwd)
    
    run "./dsh" <<EOF
cd
pwd
exit
EOF

    [[ "$output" == *"$current_dir"* ]]
}

@test "cd with invalid directory shows error" {
    run "./dsh" <<EOF
cd /nonexistent_directory_12345
exit
EOF
    
    [[ "$output" == *"cd: No such file or directory"* ]]
}

@test "Run external command - echo" {
    run "./dsh" <<EOF
echo "Hello, World!"
exit
EOF

    [[ "$output" == *"Hello, World!"* ]]
}

@test "Run external command - ls" {
    run "./dsh" <<EOF
ls -la
exit
EOF

    [[ "$output" == *"."* ]]
    [[ "$output" == *".."* ]]
}

@test "Command with multiple arguments" {
    run "./dsh" <<EOF
echo arg1 arg2 arg3
exit
EOF

    [[ "$output" == *"arg1 arg2 arg3"* ]]
}

@test "Command with quoted arguments preserves spaces" {
    run "./dsh" <<EOF
echo "  spaces   in    quotes  "
exit
EOF

    [[ "$output" == *"  spaces   in    quotes  "* ]]
}

@test "Handles nonexistent commands gracefully" {
    run "./dsh" <<EOF
nonexistentcommand123
exit
EOF

    [[ "$output" == *"nonexistentcommand123: No such file or directory"* ]]

    [ "$status" -eq 0 ]
}

@test "Command with extremely long input doesn't crash" {
    long_arg=$(printf '%200s' | tr ' ' 'a')
    
    run "./dsh" <<EOF
echo $long_arg
exit
EOF

    [ "$status" -eq 0 ]

    [[ "$output" == *"$long_arg"* ]]
}

@test "Handles commands with leading/trailing spaces" {
    run "./dsh" <<EOF
   echo   "trimmed"   
exit
EOF

    [[ "$output" == *"trimmed"* ]]
}

@test "Multiple commands execute sequentially" {
    run "./dsh" <<EOF
echo first
echo second
echo third
exit
EOF

    [[ "$output" == *"first"*"second"*"third"* ]]
}

@test "Handles input with mixed quotes" {
    run "./dsh" <<EOF
echo "quoted" unquoted "more quotes"
exit
EOF

    [[ "$output" == *"quoted unquoted more quotes"* ]]
}

@test "Handles Ctrl+D (EOF)" {
    run bash -c 'echo -e "pwd\n" | ./dsh'

    [ "$status" -eq 0 ]

    [[ "$output" == *"$(pwd)"* ]]
    [[ "$output" == *"cmd loop returned 0"* ]]
}

@test "Handles unusual characters in arguments" {
    run "./dsh" <<EOF
echo "!@#$%^&*()-_=+[]{}\\|;:,.<>/?"
exit
EOF

    [[ "$output" == *"!@#$%^&*()-_=+[]{}\\|;:,.<>/?"* ]]
}

@test "Return code from external command is handled properly" {
    run "./dsh" <<EOF
false
exit
EOF

    [ "$status" -eq 0 ]

    [[ "$output" == *"dsh4> "* ]]
}

@test "Can execute commands from different directories" {
    run "./dsh" <<EOF
cd /tmp
pwd
cd /usr
pwd
exit
EOF

    [[ "$output" == *"/tmp"* ]]
    [[ "$output" == *"/usr"* ]]
}

@test "Spaces in directory names are handled correctly" {
    mkdir -p "/tmp/dsh test dir with spaces"
    
    run "./dsh" <<EOF
cd "/tmp/dsh test dir with spaces"
pwd
exit
EOF

    [[ "$output" == *"/tmp/dsh test dir with spaces"* ]]

    rmdir "/tmp/dsh test dir with spaces"
}

@test "Simple pipe works: ls | grep make" {
    run "./dsh" <<EOF
ls | grep make
exit
EOF

    [[ "$output" == *"makefile"* ]]
}

@test "Multi-pipe works: ls | grep .c | grep dsh" {
    run "./dsh" <<EOF
ls | grep .c | grep dsh
exit
EOF

    [[ "$output" == *"dsh_cli.c"* ]]
    [[ "$output" == *"dshlib.c"* ]]
}

@test "Pipe with commands that fail gracefully" {
    run "./dsh" <<EOF
ls nonexistentdir | grep test
exit
EOF

    [[ "$output" == *"No such file or directory"* ]]
    [[ "$status" -eq 0 ]]
}

@test "Pipe with whitespace around pipe symbol" {
    run "./dsh" <<EOF
ls    |    grep   .c
exit
EOF

    [[ "$output" == *".c"* ]]
}

@test "Complex pipeline with multiple commands" {
    run "./dsh" <<EOF
ls -la | grep .c | wc -l
exit
EOF

    [[ "$output" =~ [0-9]+ ]]
}

@test "Mixing built-in and external commands with pipes" {
    run "./dsh" <<EOF
echo "Current directory:" | cat
pwd | grep /
exit
EOF

    [[ "$output" == *"Current directory:"* ]]
    [[ "$output" == *"/"* ]]
}

@test "Maximum number of pipes" {
    run "./dsh" <<EOF
echo hello | wc -c
exit
EOF
    
    echo "Test output: $output"
    echo "Status: $status"
    
    [[ "$output" =~ [0-9]+ ]]
    [[ "$status" -eq 0 ]]
}

@test "Exceeding maximum pipe limit shows error" {
    # Exceeding CMD_MAX (8 commands) should show an error
    run "./dsh" <<EOF
echo test | cat | cat | cat | cat | cat | cat | cat | cat | cat
exit
EOF

    [[ "$output" == *"error: piping limited to 8 commands"* ]]
}

@test "Pipes with empty commands are skipped" {
    run bash -c 'echo -e "ls |\n" | ./dsh'
    
    [[ "$output" == *"warning: no commands provided"* ]]
}

@test "Quoted pipes are treated as arguments, not pipeline separators" {
    run "./dsh" <<EOF
echo "This | is | not | a | pipe"
exit
EOF

    [[ "$output" == *"This | is | not | a | pipe"* ]]
}

@test "Pipe output to file using redirection" {
    # Clean up any previous test file
    if [ -f /tmp/pipe_test_output.txt ]; then
        rm /tmp/pipe_test_output.txt
    fi
    
    run "./dsh" <<EOF
ls | grep .c > /tmp/pipe_test_output.txt
cat /tmp/pipe_test_output.txt
exit
EOF

    # Verify that the file was created with the output
    [[ -f /tmp/pipe_test_output.txt ]]
    
    file_content=$(cat /tmp/pipe_test_output.txt)
    [[ "$file_content" == *".c"* ]]
    
    # Clean up
    rm /tmp/pipe_test_output.txt
}

@test "Remote Shell: Server starts successfully" {
    # Start server and save its PID
    ./dsh -s -p 5555 &
    server_pid=$!
    sleep 1

    # Check if process is still running
    ps -p $server_pid > /dev/null
    [ $? -eq 0 ]

    # Clean up
    kill $server_pid
}

@test "Remote Shell: Client connects to server" {
    ./dsh -s -p 5556 &
    sleep 1
    
    run timeout 1 ./dsh -c -p 5556 <<EOF
exit
EOF
    
    pkill -f "./dsh -s -p 5556"
    
    [ "$status" -eq 0 ]
    [[ "$output" == *"dsh4>"* ]]
}

@test "Remote Shell: Basic command execution" {
    ./dsh -s -p 5557 &
    sleep 1
    
    run timeout 1 ./dsh -c -p 5557 <<EOF
echo "Hello Remote Shell"
exit
EOF
    
    pkill -f "./dsh -s -p 5557"
    
    [ "$status" -eq 0 ]
    [[ "$output" == *"Hello Remote Shell"* ]]
}

@test "Remote Shell: CD command works" {
    ./dsh -s -p 5558 &
    sleep 1
    
    mkdir -p /tmp/dsh-remote-test
    
    # Try cd command
    run timeout 1 ./dsh -c -p 5558 <<EOF
cd /tmp/dsh-remote-test
pwd
exit
EOF
    
    pkill -f "./dsh -s -p 5558"
    
    rmdir /tmp/dsh-remote-test
    
    [ "$status" -eq 0 ]
    [[ "$output" == *"/tmp/dsh-remote-test"* ]]
}

@test "Remote Shell: Pipeline execution" {
    ./dsh -s -p 5559 &
    sleep 1
    
    # Run pipeline command
    run timeout 1 ./dsh -c -p 5559 <<EOF
echo "test line" | grep line
exit
EOF
    
    pkill -f "./dsh -s -p 5559"
    
    [ "$status" -eq 0 ]
    [[ "$output" == *"test line"* ]]
}

@test "Remote Shell: Stop server command" {
    ./dsh -s -p 5560 &
    sleep 1
    
    # Send stop-server command
    run timeout 1 ./dsh -c -p 5560 <<EOF
stop-server
EOF
    
    [ "$status" -eq 0 ]
}


@test "Remote Shell: Error handling - invalid command" {
    ./dsh -s -p 5562 &
    sleep 1
    
    # Run invalid command
    run timeout 1 ./dsh -c -p 5562 <<EOF
nonexistentcommand123
exit
EOF
    
    pkill -f "./dsh -s -p 5562"
    
    [ "$status" -eq 0 ]
    [[ "$output" == *"No such file or directory"* ]]
}

@test "Remote Shell: Error handling - invalid directory" {
    ./dsh -s -p 5563 &
    sleep 1
    
    # Try to cd to invalid directory
    run timeout 1 ./dsh -c -p 5563 <<EOF
cd /nonexistent_directory_12345
exit
EOF
    
    pkill -f "./dsh -s -p 5563"
    
    [ "$status" -eq 0 ]
    [[ "$output" == *"No such file or directory"* ]]
}

@test "Remote Shell: Large output handling" {
    ./dsh -s -p 5564 &
    sleep 1
    
    # Generate large output
    run timeout 1 ./dsh -c -p 5564 <<EOF
yes "test" | head -1000
exit
EOF
    
    pkill -f "./dsh -s -p 5564"
    
    [ "$status" -eq 0 ]
    [[ "${#output}" -gt 4000 ]]
}