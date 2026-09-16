#include "lab.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 1024

struct addrinfo *getgetaddrinfo(const char *server, const char *port)
{
  int s = 0;
  struct addrinfo *result;
  s = getaddrinfo(server, port, NULL, &result);
  if (s != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    result = NULL;
  }

  return result;
}

int connectconnect(struct addrinfo* address)
{
  int s = 0;
  struct addrinfo *rp;
  for (rp = address; rp != NULL; rp = rp->ai_next)
  {
    s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (s == -1) continue;
    if(connect(s, rp->ai_addr, rp->ai_addrlen) != -1) break;
    close(s);
  }

  freeaddrinfo(address);

  if(rp == NULL)
  {
    fprintf(stderr, "Could not connect\n");
    return(0);
  }
  return s;
}

int read_smtp(int socket, char* buffer, size_t bufferlen)
{
  ssize_t bytes_received = recv(socket, buffer, bufferlen - 1, 0);
  if(bytes_received > 0)
  {
    buffer[bytes_received-1] = '\0';
    printf("Message: %s\n", buffer);
    return 0;
  }
  else if (bytes_received == 0)
  {
    printf("Connection closed by peer\n");
  }
  else 
  {
    printf("recv failed");
  }

  return 1;
}

ssize_t write_smtp(int socket, char* message)
{
  printf("%s", message);
  return send(socket, message, strlen(message), 0);
}

int start_session(const char* server, const char* port)
{
  struct addrinfo* address;
  int socket;

  address = getgetaddrinfo(server, port);
  socket = connectconnect(address);

  return socket;
}

bool check_status_code(const char* message_in, const char* code_expected)
{
  return (strncmp(message_in, code_expected, 3) == 0);
}

bool check_last_line(const char* message_in)
{
  return (message_in[3] == ' ');
}

int read_line(read_msg_callback read_msg, int socket, char* buffer, size_t bufferlen)
{
  return read_msg(socket, buffer, bufferlen);
}

int read_reply(read_msg_callback read_msg, int socket, char* buffer, size_t bufferlen)
{
  bool last_line = false;
  int ret = 0;
  while(!last_line && ((ret = read_line(read_msg, socket, buffer, bufferlen)) == 0))
  {
    last_line = check_last_line(buffer);
  }

  return ret;
}

ssize_t write_line(write_msg_callback write_msg, int socket, char* message)
{
  return write_msg(socket, message);
}

bool send_cmd(write_msg_callback write_msg, read_msg_callback read_msg, int socket, char* buffer, size_t bufferlen, char* expected_status)
{
  if(write_line(write_msg, socket, buffer) > 0)
  {
    read_reply(read_msg, socket, buffer, bufferlen);
  }
  return check_status_code(buffer, expected_status);
}

char* prep_msg(char* raw_msg, char* command, char* buffer)
{
  char* next_pos = buffer;
  next_pos = stpcpy(next_pos, command);
  next_pos = stpcpy(next_pos, raw_msg);
  next_pos = stpcpy(next_pos, "\r\n");
  return next_pos;
}

char* prep_helo(char* raw_msg, char* buffer)
{
  return prep_msg(raw_msg, "HELO ", buffer);
}

char* prep_mail_from(char* raw_msg, char* buffer)
{
  return prep_msg(raw_msg, "MAIL FROM:", buffer);
}

char* prep_rcpt_to(char* raw_msg, char* buffer)
{
  return prep_msg(raw_msg, "RCPT TO:", buffer);
}

char* prep_data(char* buffer)
{
  return stpcpy(buffer, "DATA\r\n");
}

char* prep_bye(char* buffer)
{
  return stpcpy(buffer, "QUIT\r\n");
}

bool send_helo(write_msg_callback write_msg, read_msg_callback read_msg, int socket, char* helo_host)
{
  char buffer[BUFFER_SIZE];
  prep_helo(helo_host, buffer);
  return send_cmd(write_msg, read_msg, socket, buffer, sizeof(buffer), "250");
}

bool send_mail_from(write_msg_callback write_msg, read_msg_callback read_msg, int socket, char* mail_from)
{
  char buffer[BUFFER_SIZE];
  prep_mail_from(mail_from, buffer);
  return send_cmd(write_msg, read_msg, socket, buffer, sizeof(buffer), "250");
}

bool send_rcpt_to(write_msg_callback write_msg, read_msg_callback read_msg, int socket, char* rcpt_to)
{
  char buffer[BUFFER_SIZE];
  prep_rcpt_to(rcpt_to, buffer);
  return send_cmd(write_msg, read_msg, socket, buffer, sizeof(buffer), "250");
}

bool send_data(write_msg_callback write_msg, read_msg_callback read_msg, int socket)
{
  char buffer[BUFFER_SIZE];
  prep_data(buffer);
  return send_cmd(write_msg, read_msg, socket, buffer, sizeof(buffer), "354");
}

bool send_bye(write_msg_callback write_msg, read_msg_callback read_msg, int socket)
{
  char buffer[BUFFER_SIZE];
  prep_bye(buffer);
  return send_cmd(write_msg, read_msg, socket, buffer, sizeof(buffer), "221");
}

bool send_body(write_msg_callback write_msg, read_msg_callback read_msg, int socket, char* subject, char* body)
{
  // Write the subject if it exists
  if (strcmp(subject, "") != 0)
  {
    write_line(write_msg, socket, "Subject: ");
    write_line(write_msg, socket, subject);
    write_line(write_msg, socket, "\r\n");
  }

  // Split the message body by line
  char* current_line = body;
  while(current_line != NULL)
  {
    char* next_line = strchr(current_line, '\n');
    
    if (next_line != NULL)
    {
      *next_line = '\0';
    }

    // Dot stuff if necessary
    if(current_line[0] == '.')
    {
      write_line(write_msg, socket, ".");
    }

    // Send the message and a new line.
    write_line(write_msg, socket, current_line);
    write_line(write_msg, socket, "\r\n");
    if(next_line != NULL)
    {
      *next_line = '\n';
      current_line = next_line + 1;
    } else {
      current_line = NULL;
    }
  }

  write_line(write_msg, socket, ".\r\n");
  char buffer[BUFFER_SIZE];
  read_reply(read_msg, socket, buffer, sizeof(buffer));
  return check_status_code(buffer, "250");
}

int run_session(write_msg_callback write_msg, read_msg_callback read_msg, char* server, char* port, char* host, char* email_from, char* email_to, char* subject, char* body)
{
  int socket = start_session(server, port);
  if(socket == 0)
  {
    fprintf(stderr, "Error binding socket\n");
    return(2);
  }

  bool passed;
  char buffer[BUFFER_SIZE];
  read_reply(read_msg, socket, buffer, sizeof(buffer));
  passed = check_status_code(buffer, "220");
  if (passed)
  {
    passed = send_helo(write_msg, read_msg, socket, host);
  } else {
    fprintf(stderr, "Error sending HELO\n");
    return 2;
  }

  if (passed)
  {
    passed = send_mail_from(write_msg, read_msg, socket, email_from);
  } else {
    fprintf(stderr, "Error sending MAIL FROM\n");
    return 2;
  }

  if (passed)
  {
    passed = send_rcpt_to(write_msg, read_msg, socket, email_to);
  } else {
    fprintf(stderr, "Error sending RCPT TO\n");
    return 2;
  }

  if (passed) 
  {
    passed = send_data(write_msg, read_msg, socket);
  } else {
    fprintf(stderr, "Error sending DATA\n");
    return 2;
  }

  if (passed)
  {
    passed = send_body(write_msg, read_msg, socket, subject, body);
  } else {
    fprintf(stderr, "Error sending message body\n");
    return 2;
  }
  
  if (passed)
  {
    passed = send_bye(write_msg, read_msg, socket);
  } else {
    fprintf(stderr, "Error sending QUIT");
    return 2;
  }

  return 0;
}
