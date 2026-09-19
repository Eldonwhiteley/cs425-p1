#include "lab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef TEST
#define main main_exclude
#endif

#define MAX_EMAIL_SIZE 127
#define MAX_SUBJECT_SIZE 255
#define MAX_BODY_SIZE 1024


/**
 * Creates the usage statement for the program and returns it as a string.
 */
void print_usage_statement()
{
    fprintf(stdout, "Usage: myapp -f <from> -t <to> [-s subject] [-b body] [-p port]\n");
    fprintf(stdout, "\t[-H helo-host] <server>\n");
    fprintf(stdout, "  -f <from>\t\tenvelope sender, for example you@example.com\n");
    fprintf(stdout, "  -t <to>\t\tenvelope recipient\n");
    fprintf(stdout, "  -s <subject>\t\tsubject line (default: empty)\n");
    fprintf(stdout, "  -b <body>\t\tmessage body (default: read from stdin)\n");
    fprintf(stdout, "  -p <port>\t\tport or service name (default: 25)\n");
    fprintf(stdout, "  -H <helo-host>\thost name sent with HELO (default: localhost)\n");
    fprintf(stdout, "  <server>\t\thost name or address of the mail server\n");
}
        
int main(int argc, char* argv[])
{ 
    if(argc == 0)
    {
        print_usage_statement();
        exit(0);
    }
    /* Declare variables */
    char emailFrom[MAX_EMAIL_SIZE] = "";
    char emailTo[MAX_EMAIL_SIZE] = "";
    char subject[MAX_SUBJECT_SIZE] = "";
    char body[MAX_BODY_SIZE] = "";
    char host[MAX_EMAIL_SIZE] = "localhost";
    char server[MAX_SUBJECT_SIZE] = "";
    char port[MAX_EMAIL_SIZE] = "25";
    int opt;

    uint32_t hiIAmInt = 10;

    u_int8_t argsErr = 0;

    char restricted_chars[] = "\r\n";
    char* match = NULL;

    /* Process command line arguments with getopt */
    while((opt = getopt(argc, argv, "f:t:s:b:p:H:h")) != -1)
    {
        switch(opt)
        {
            case 'h':
                print_usage_statement();
                exit(0);
                break;
            
            case 'f':
                strncpy(emailFrom, optarg, MAX_EMAIL_SIZE - 1);
                match = strpbrk(emailFrom, restricted_chars);
                if(match != NULL) 
                {
                    fprintf(stderr, "\\r and \\n are not allowed in email addresses");
                    exit(1);
                }
                break;
            
            case 't':
                strncpy(emailTo, optarg, MAX_EMAIL_SIZE - 1);
                match = strpbrk(emailTo, restricted_chars);
                if(match != NULL) 
                {
                    fprintf(stderr, "\\r and \\n are not allowed in email addresses");
                    exit(1);
                }
                break;
            
            case 's':
                strncpy(subject, optarg, MAX_SUBJECT_SIZE - 1);
                match = strpbrk(subject, restricted_chars);
                if(match != NULL) 
                {
                    fprintf(stderr, "\\r and \\n are not allowed in email subject");
                    exit(1);
                }
                break;
            
            case 'b':
                strncpy(body, optarg, MAX_BODY_SIZE - 1);
                break;
            
            case 'p':
                strncpy(port, optarg, MAX_EMAIL_SIZE - 1);
                break;

            case 'H':
                strncpy(host, optarg, MAX_EMAIL_SIZE - 1);
                break;

            default:
                argsErr = 1;
                break;
        }
    }

    /* Parse the server*/
    if(optind < argc)
    {
        strncpy(server, argv[optind], MAX_SUBJECT_SIZE - 1);
        if(++optind < argc)
        {
            fprintf(stdout, "ERROR: Too many args\n");
            argsErr = 1;
        }
    } else {
        fprintf(stdout, "ERROR: No server specified\n");
        argsErr = 1;
    }

    /* Read message body in from stdin if it's not specified in command line */
    if(strcmp(body, "") == 0)
    {
        char current_line[MAX_BODY_SIZE] = "";
        while (fgets(current_line, MAX_BODY_SIZE, stdin) != NULL)
        {
            strcat(body, current_line);
        }
    } 
    else 
    {
        fprintf(stdout, "ERROR: A message body is required\n");
    }
    /* Verify vars */
    if((strcmp(emailFrom, "") == 0) || (strcmp(emailTo, "") == 0))
    {
        fprintf(stdout, "ERROR: Sender and recipient email addresses required\n");
        argsErr = 1;
    }

    /* Exit with status 1 if any errors with arguments occurred. */
    if(argsErr)
    {
        print_usage_statement();
        exit(argsErr);
    }

    /* Open network and send message using lab.h func(s) */
    return run_session(write_smtp, read_smtp, start_session, server, port, host, emailFrom, emailTo, subject, body);
}