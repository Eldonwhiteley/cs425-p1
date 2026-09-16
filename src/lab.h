#ifndef LAB_H
#define LAB_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdbool.h>

typedef int (*read_msg_callback)(int, char*, size_t);

typedef ssize_t (*write_msg_callback)(int, char*);

/**
 * Get and return an addrinfo struct created by getaddrinfo()
 * 
 * server - [in] the server to resolve
 * port - [in] the port to use
 * 
 * return addrinfo struct holding the address of the server
 */
struct addrinfo* getgetaddrinfo(const char* server, const char* port);

/**
 * Connect the client to the server :)
 * 
 * address - [in] addrinfo struct containing the address of the server
 * 
 * return 0 on error, otherwise file descriptor for the socket.
 */
int connectconnect(struct addrinfo* address);

/**
 * Reads a message from the SMTP server and returns it as a string.
 * 
 * socket - [in] the socket file descriptor given by connectconnect()
 * buffer - [out] the message from the socket
 * 
 * return 0 on success, 1 on error
 */
int read_smtp(int socket, char* buffer, size_t bufferlen);

/**             
 * Sends a message to the SMTP server.
 * 
 * socket - [in] the socket file descriptor given by connectconnect()
 * message - [in] the string message to send.
 * 
 * return -1 on error, otherwise the size of the message sent
 */
ssize_t write_smtp(int socket, char* message);

/**
 * Get address info and connect to the server with one command
 * 
 * server - [in] the server to connect to
 * port - [in] the port to use for the connection
 * 
 * return 0 on error, otherwise file descriptor for the socket
 */
int start_session(const char* server, const char* port);

/**
 * Read a message as a string and check the status code.
 * 
 * message_in - [in] the message to check
 * code_expected - [in] the expected status code
 * 
 * return true if the status code in the message matches expected
 */
bool check_status_code(const char* message_in, const char* code_expected);

/**
 * Read a message as a string and check if it is the last line of the message.
 * 
 * message_in - [in] the message to check
 * 
 * return true if the line is a last line
 */
bool check_last_line(const char* message_in);

/**
 * A layer 2 function to read a message line.
 * 
 * read_msg - [in] a callback function that fills the buffer with a message read
 * socket - [in] socket for reading from an smtp connection
 * buffer - [in] buffer to write into
 * bufferlen - [in] the maximum size of the buffer
 * 
 * return 0 on success, 1 on error
 */
int read_line(read_msg_callback read_msg, int socket, char* buffer, size_t bufferlen);

/**
 * Reads an entire reply by reading a line until a message is a last line
 * 
 * read_msg - [in] a callback function that fills the buffer with a message read
 * socket - [in] socket for reading from an smtp connection
 * buffer - [in] buffer to write into
 * bufferlen - [in] the maximum size of the buffer
 * 
 * return 0 on success, 1 on error
 */
int read_reply(read_msg_callback read_msg, int socket, char* buffer, size_t bufferlen);

/**
 * Layer 2 function to write a message line.
 * 
 * write_msg - [in] a callback function that writes the message stored in message
 * socket - [in] socket for writing to an smtp connection
 * message - [in] message to send
 * 
 * return -1 on error otherwise number of bytes written
 */
ssize_t write_line(write_msg_callback write_msg, int socket, char* message);

/**
 * Send a command and check its status code.
 * 
 * write_msg - [in] a callback function that writes a message
 * read_msg - [in] a callback function that reads a message
 * socket - [in] socket for reading from an smtp connection
 * buffer - [in/out] buffer for reading and writing a message
 * bufferlen - [in] maximum size of the buffer
 * expected_status - [in] status we expect to return from the sent message
 * 
 * return true if the status is good, false otherwise
 */
bool send_cmd(write_msg_callback write_msg, read_msg_callback read_msg, int socket, char* buffer, size_t bufferlen, char* expected_status);

/**
 * Prepare a message by taking the raw message, adding the proper command to the beginning
 * and terminating with a <CRLF>
 * 
 * raw_msg - [in] message to prepare
 * command - [in] command type (HELO, MAIL FROM, RCPT TO)
 * buffer - [out] buffer to save the message to
 * 
 * returns a pointer to the terminating null byte of the buffer
 */
char* prep_msg(char* raw_msg, char* command, char* buffer);

/**
 * Prepare a HELO message
 * 
 * raw_msg - [in] message to prepare
 * buffer - [out] buffer to save the message to
 * 
 * returns a pointer to the terminating null byte of the buffer
 */
char* prep_helo(char* raw_msg, char* buffer);

/**
 * Prepare a MAIL FROM message
 * 
 * raw_msg - [in] message to prepare
 * buffer - [out] buffer to save the message to
 * 
 * returns a pointer to the terminating null byte of the buffer
 */
char* prep_mail_from(char* raw_msg, char* buffer);

/**
 * Prepare an RCPT TO message
 * 
 * raw_msg - [in] message to prepare
 * buffer - [out] buffer to save the message to
 * 
 * returns a pointer to the terminating null byte of the buffer
 */
char* prep_rcpt_to(char* raw_msg, char* buffer);

/**
 * Prepare a DATA message
 * 
 * buffer - [out] buffer to save the message to
 * 
 * returns a pointer to the terminating null byte of the buffer
 */
char* prep_data(char* buffer);

/**
 * Prepare a BYE message
 * 
 * buffer - [out] buffer to save the message to
 * 
 * returns a pointer to the terminating null byte of the buffer
 */
char* prep_bye(char* buffer);

/**
 * Dot stuff a message line that consists of only a '.'
 */
char* dot_stuff(char* line);

bool send_helo(write_msg_callback write_msg, read_msg_callback read_msg, int socket, char* helo_host);

bool send_mail_from(write_msg_callback write_msg, read_msg_callback read_msg, int socket, char* mail_from);

bool send_rcpt_to(write_msg_callback write_msg, read_msg_callback read_msg, int socket, char* rcpt_to);

bool send_data(write_msg_callback write_msg, read_msg_callback read_msg, int socket);

bool send_bye(write_msg_callback write_msg, read_msg_callback read_msg, int socket);

bool send_body(write_msg_callback write_msg, read_msg_callback read_msg, int socket, char* subject, char* body);

int run_session(write_msg_callback write_msg, read_msg_callback read_msg, char* server, char* port, char* host, char* email_from, char* email_to, char* subject, char* body);

#endif // LAB_H
