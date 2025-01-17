#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
int reverse_string(char *, int, int);
int print_words(char *, int, int);
int search_replace(int);


int setup_buff(char *buff, char *user_str, int len){
    //TODO: #4:  Implement the setup buff as per the directions
    int user_str_len = 0;
    char *ptr = user_str;
    char *buff_ptr = buff;
    int in_space = 1;

    while (*ptr) {
        if (user_str_len >= len) return -1;
        if (*ptr == ' ' || *ptr == '\t') {
            if (!in_space) {  //Checks if you are not is a sequence of spaces
                *buff_ptr++ = ' ';
                user_str_len++;
            }
            in_space = 1;  //Currently in a sequence of spaces
        } else {
            *buff_ptr++ = *ptr;
            user_str_len++;
            in_space = 0;
        }
        ptr++;
    }

    if (buff_ptr > buff && *(buff_ptr - 1) == ' ') { //Removes remaining spaces
        buff_ptr--;
        user_str_len--;
    }

    while (buff_ptr < buff + len) {
        *buff_ptr++ = '.';
    }

    return user_str_len;
}

void print_buff(char *buff, int len){
    printf("Buffer:  [");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    putchar(']');
    putchar('\n');
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

int count_words(char *buff, int buff_len, int str_len){
    if (str_len > buff_len) {
        return -1;
    }

    int word_count = 0;
    int in_word = 0;
    char *ptr = buff;

    for (int i = 0; i < str_len; i++) {
        char c = *ptr++;
        if (c == ' ') {
            in_word = 0; //Word has finished
        } else if (!in_word) {
            word_count++;
            in_word = 1; //Word has started
        }
    }

    return word_count;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS

int print_words(char *buff, int buff_len, int str_len) {
    if (str_len > buff_len) {
        return -1;
    }

    int word_count = 0;
    int char_count = 0;
    int start = 1;
    char *ptr = buff;

    printf("Word Print\n----------\n");

    for (int i = 0; i < str_len; i++) {
        char c = *ptr++;
        if (c == ' ') {
            if (char_count > 0) {
                printf(" (%d)\n", char_count);
                char_count = 0;  //Resets count for next word
                start = 1;    //This is the start of a new word
            }
        } else {
            if (start) {
                word_count++;
                printf("%d. ", word_count);
                start = 0;
            }
            putchar(c);
            char_count++;
        }
    }

    if (char_count > 0) {
        printf(" (%d)\n", char_count);
    }

    printf("\n");
    printf("Number of words returned: %d\n", word_count);

    return word_count;
}

int reverse_string(char *buff, int buff_len, int str_len) {
    if (str_len > buff_len) {
        return -1;
    }

    char *start = buff;
    char *end = buff + str_len - 1;

    while (start < end) {
            char temp = *start;
            *start++ = *end;
            *end-- = temp;
    }
    
    return 0;
}

int search_replace(int argc) {
    if (argc < 5) {
        printf("Missing arguments for -x\n");
        return -1;
    }

    printf("Not Implemented!\n");
    return 0;
}



int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    /*
       The first part (argc < 2) checks if there are at least two arguments. If that is true,
       the program displays the usage template and then exits.
       The second part (*argv[1] != '-') checks that the second argument starts with -. If the second
       argument does not start with -, the program displays the usage and exits.
       If argv[1] does not exist, the program does not know what to do with the input, automatically
       giving the user a guide to use the program properly.
    */
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    /*
       The purpose of the if statement is to check whether the user has provided a third argument or not.
       If the third argument did not exist and the if statement did not either, then the program would try
       to use a null pointer, which would likely cause a segmentation fault.
    */
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3

    buff = (char *)malloc(BUFFER_SZ);
    if (!buff) {
        perror("Memory allocation failed");
        exit(99);
    }


    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        free(buff);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;
        case 'r':
            rc = reverse_string(buff, BUFFER_SZ, user_str_len);
            if (rc < 0) {
                printf("Error reversing string, rc = %d", rc);
                exit(2);
            }
            break;
        case 'w':
            rc = print_words(buff, BUFFER_SZ, user_str_len);
            if (rc < 0) {
                printf("Error: Buffer error\n");
                exit(2);
            }
            break;
        case 'x':
            rc = search_replace(argc);
            if (rc < 0) {
                printf("Error: missing arguments\n");
                exit(2);
            }
            break;
        default:
            usage(argv[0]);
            free(buff);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff,BUFFER_SZ);
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          PLACE YOUR ANSWER HERE

/*
   The cause for taking both the buffer as well as the length is due to possible future
   implementations, which could use a different buffer size.
   There are also other advantages to this, such as making a function more readable, or easier to debug.
*/
