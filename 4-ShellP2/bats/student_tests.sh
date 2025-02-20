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
    
    [[ "$output" == *"dsh2> "* ]]
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
echo !@#$%^&*()-_=+[]{}\\|;:,.<>/?
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

    [[ "$output" == *"dsh2> "* ]]
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