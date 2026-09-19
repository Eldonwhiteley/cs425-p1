#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "harness/unity.h"
#include "../src/lab.h"

#define BUFFER_SIZE 1024

void setUp(void) {
  printf("Setting up tests...\n");
}

void tearDown(void){
  printf("Tearing down tests...\n");
}

int fakeRead(int return_code, char* buffer, size_t bufferlen)
{
  strcpy(buffer, buffer);
  (void) bufferlen;
  return return_code;
}

int read250(int return_code, char* buffer, size_t bufferlen)
{
  strcpy(buffer, "250 Message\r\n");
  (void) bufferlen;
  return return_code;
}

int read354(int return_code, char* buffer, size_t bufferlen)
{
  strcpy(buffer, "354 Message\r\n");
  (void) bufferlen;
  return return_code;
}

int read221(int return_code, char* buffer, size_t bufferlen)
{
  strcpy(buffer, "221 Message\r\n");
  (void) bufferlen;
  return return_code;
}

int fake_session(const char* socket, const char* port)
{
  (void) socket;
  return atoi(port);
}

ssize_t fakeWrite(int return_code, char* buffer)
{
  strcpy(buffer, buffer);
  return return_code;
}

void test_check_status_code(void)
{
  char* status_msg = "220 Good stuff :)";
  char* good_status = "220";
  char* bad_status = "123";
  char* truncated_status = "220030";

  TEST_ASSERT_TRUE(check_status_code(status_msg, good_status));
  TEST_ASSERT_FALSE(check_status_code(status_msg, bad_status));
  TEST_ASSERT_TRUE(check_status_code(status_msg, truncated_status));
}

void test_check_last_line(void)
{
  char* not_last_line = "123-I'm not a last line";
  char* last_line = "123 I am a last line";
  char* maybe_last_line = "I might be a last line";

  TEST_ASSERT_FALSE(check_last_line(not_last_line));
  TEST_ASSERT_TRUE(check_last_line(last_line));
  TEST_ASSERT_FALSE(check_last_line(maybe_last_line));
}

void test_check_format(void)
{
  char* good_format = "123 I'm good format";
  char* good_format2 = "456-I'm also good format";
  char* bad_format = "I'm bad format";
  char* bad_format2 = "1234 I'm also bad format";

  TEST_ASSERT_TRUE(check_format(good_format));
  TEST_ASSERT_TRUE(check_format(good_format2));
  TEST_ASSERT_FALSE(check_format(bad_format));
  TEST_ASSERT_FALSE(check_format(bad_format2));
}

void test_read_line(void)
{
  char* buffer = "";

  TEST_ASSERT_TRUE((read_line(fakeRead, 0, buffer, 0) == 0));
  TEST_ASSERT_TRUE((read_line(fakeRead, 1, buffer, 0) == 1));
}

void test_read_reply(void)
{
  char* good_format = "123 This is a properly formatted response";
  char* bad_format = "This response isn't formatted properly";

  TEST_ASSERT_TRUE(read_reply(fakeRead, 0, good_format, 0) == 0);
  TEST_ASSERT_TRUE(read_reply(fakeRead, 1, good_format, 0) == 1);
  TEST_ASSERT_TRUE(read_reply(fakeRead, 0, bad_format, 0) == 1);
}

void test_write_line(void)
{
  char* buffer = "";

  TEST_ASSERT_TRUE(write_line(fakeWrite, 10, buffer) == 10);
  TEST_ASSERT_TRUE(write_line(fakeWrite, -1, buffer) == -1);
}

void test_send_cmd(void)
{
  char* good_format = "123 This is a properly formatted message";
  char* bad_format = "This message is not formatted properly";

  TEST_ASSERT_TRUE(send_cmd(fakeWrite, fakeRead, 0, good_format, 0, "123"));
  TEST_ASSERT_FALSE(send_cmd(fakeWrite, fakeRead, 0, bad_format, 0, "123"));
  TEST_ASSERT_FALSE(send_cmd(fakeWrite, fakeRead, 1, bad_format, 0, "123"));
}

void test_prep_msg(void)
{
  char buffer[BUFFER_SIZE];
  char* raw_msg = "This is a raw message";
  char* command = "COMMAND ";
  char* full_msg = "COMMAND This is a raw message\r\n";

  prep_msg(raw_msg, command, buffer);

  TEST_ASSERT_TRUE(strcmp(buffer, full_msg) == 0);
}

void test_prep_helo(void)
{
  char buffer[BUFFER_SIZE];
  char* raw_msg = "This is a raw message";
  char* full_msg = "HELO This is a raw message\r\n";

  prep_helo(raw_msg, buffer);

  TEST_ASSERT_TRUE(strcmp(full_msg, buffer) == 0);
}

void test_prep_mail_from(void)
{
  char buffer[BUFFER_SIZE];
  char* raw_msg = "example@mail.com";
  char* full_msg = "MAIL FROM:example@mail.com\r\n";

  prep_mail_from(raw_msg, buffer);

  TEST_ASSERT_TRUE(strcmp(full_msg, buffer) == 0);
}

void test_prep_rcpt_to(void)
{
  char buffer[BUFFER_SIZE];
  char* raw_msg = "example@mail.com";
  char* full_msg = "RCPT TO:example@mail.com\r\n";

  prep_rcpt_to(raw_msg, buffer);

  TEST_ASSERT_TRUE(strcmp(full_msg, buffer) == 0);
}

void test_prep_data(void)
{
  char buffer[BUFFER_SIZE];
  char* full_msg = "DATA\r\n";

  prep_data(buffer);

  TEST_ASSERT_TRUE(strcmp(full_msg, buffer) == 0);
}

void test_prep_bye(void)
{
  char buffer[BUFFER_SIZE];
  char* full_msg = "QUIT\r\n";

  prep_bye(buffer);

  TEST_ASSERT_TRUE(strcmp(full_msg, buffer) == 0);
}

void test_send_helo(void)
{
  TEST_ASSERT_TRUE(send_helo(fakeWrite, read250, 10, "helo"));
}

void test_send_mail_from(void)
{
  TEST_ASSERT_TRUE(send_mail_from(fakeWrite, read250, 10, "example@email.com"));
}

void test_send_rcpt_to(void)
{
  TEST_ASSERT_TRUE(send_rcpt_to(fakeWrite, read250, 10, "example@email.com"));
}

void test_send_data(void)
{
  TEST_ASSERT_TRUE(send_data(fakeWrite, read354, 10));
}

void test_send_bye(void)
{
  TEST_ASSERT_TRUE(send_bye(fakeWrite, read221, 10));
}

void test_send_body(void)
{
  char body[BUFFER_SIZE] = "This is a message with\n.\nmultiple lines";
  TEST_ASSERT_TRUE(send_body(fakeWrite, read250, 10, "", body));
  TEST_ASSERT_TRUE(send_body(fakeWrite, read250, 10, "This time there's a subject", body));
}

int sequential_read(int return_code, char* buffer, size_t bufferlen)
{
  char* codes[] = {"220", "250", "250", "250", "354", "250", "221"};
  static int codeIdx = 0;
  strcpy(buffer, codes[codeIdx]);
  strcat(buffer, "Message\r\n");
  ++codeIdx;
  return return_code;
}

int sequential_read_bad1(int return_code, char* buffer, size_t bufferlen)
{
  char* codes[] = {"221", "250", "250", "250", "354", "250", "221"};
  static int codeIdx = 0;
  strcpy(buffer, codes[codeIdx]);
  strcat(buffer, "Message\r\n");
  ++codeIdx;
  return return_code;
}

int sequential_read_bad2(int return_code, char* buffer, size_t bufferlen)
{
  char* codes[] = {"220", "251", "250", "250", "354", "250", "221"};
  static int codeIdx = 0;
  strcpy(buffer, codes[codeIdx]);
  strcat(buffer, "Message\r\n");
  ++codeIdx;
  return return_code;
}

int sequential_read_bad3(int return_code, char* buffer, size_t bufferlen)
{
  char* codes[] = {"220", "250", "251", "250", "354", "250", "221"};
  static int codeIdx = 0;
  strcpy(buffer, codes[codeIdx]);
  strcat(buffer, "Message\r\n");
  ++codeIdx;
  return return_code;
}

int sequential_read_bad4(int return_code, char* buffer, size_t bufferlen)
{
  char* codes[] = {"220", "250", "250", "251", "354", "250", "221"};
  static int codeIdx = 0;
  strcpy(buffer, codes[codeIdx]);
  strcat(buffer, "Message\r\n");
  ++codeIdx;
  return return_code;
}

int sequential_read_bad5(int return_code, char* buffer, size_t bufferlen)
{
  char* codes[] = {"220", "250", "250", "250", "355", "250", "221"};
  static int codeIdx = 0;
  strcpy(buffer, codes[codeIdx]);
  strcat(buffer, "Message\r\n");
  ++codeIdx;
  return return_code;
}

int sequential_read_bad6(int return_code, char* buffer, size_t bufferlen)
{
  char* codes[] = {"220", "250", "250", "250", "354", "251", "221"};
  static int codeIdx = 0;
  strcpy(buffer, codes[codeIdx]);
  strcat(buffer, "Message\r\n");
  ++codeIdx;
  return return_code;
}

int sequential_read_bad7(int return_code, char* buffer, size_t bufferlen)
{
  char* codes[] = {"220", "250", "250", "250", "354", "250", "222"};
  static int codeIdx = 0;
  strcpy(buffer, codes[codeIdx]);
  strcat(buffer, "Message\r\n");
  ++codeIdx;
  return return_code;
}

void test_run_session(void)
{
  TEST_ASSERT_TRUE(run_session(fakeWrite, sequential_read, fake_session, "whatever", "5", "localhost", "example@email.com", "example2@email.com", "", "body") == 0);
  TEST_ASSERT_TRUE(run_session(fakeWrite, sequential_read, fake_session, "whatever", "0", "localhost", "example@email.com", "example2@email.com", "", "body") == 2);
  TEST_ASSERT_TRUE(run_session(fakeWrite, sequential_read_bad1, fake_session, "whatever", "5", "localhost", "example@email.com", "example2@email.com", "", "body") == 2);
  TEST_ASSERT_TRUE(run_session(fakeWrite, sequential_read_bad2, fake_session, "whatever", "5", "localhost", "example@email.com", "example2@email.com", "", "body") == 2);
  TEST_ASSERT_TRUE(run_session(fakeWrite, sequential_read_bad3, fake_session, "whatever", "5", "localhost", "example@email.com", "example2@email.com", "", "body") == 2);
  TEST_ASSERT_TRUE(run_session(fakeWrite, sequential_read_bad4, fake_session, "whatever", "5", "localhost", "example@email.com", "example2@email.com", "", "body") == 2);
  TEST_ASSERT_TRUE(run_session(fakeWrite, sequential_read_bad5, fake_session, "whatever", "5", "localhost", "example@email.com", "example2@email.com", "", "body") == 2);
  TEST_ASSERT_TRUE(run_session(fakeWrite, sequential_read_bad6, fake_session, "whatever", "5", "localhost", "example@email.com", "example2@email.com", "", "body") == 2);
  TEST_ASSERT_TRUE(run_session(fakeWrite, sequential_read_bad7, fake_session, "whatever", "5", "localhost", "example@email.com", "example2@email.com", "", "body") == 2);

}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_check_status_code);
  RUN_TEST(test_check_last_line);
  RUN_TEST(test_check_format);
  RUN_TEST(test_read_line);
  RUN_TEST(test_read_reply);
  RUN_TEST(test_write_line);
  RUN_TEST(test_send_cmd);
  RUN_TEST(test_prep_msg);
  RUN_TEST(test_prep_helo);
  RUN_TEST(test_prep_mail_from);
  RUN_TEST(test_prep_rcpt_to);
  RUN_TEST(test_prep_data);
  RUN_TEST(test_prep_bye);
  RUN_TEST(test_send_helo);
  RUN_TEST(test_send_mail_from);
  RUN_TEST(test_send_rcpt_to);
  RUN_TEST(test_send_data);
  RUN_TEST(test_send_bye);
  RUN_TEST(test_send_body);
  RUN_TEST(test_run_session);
  return UNITY_END();
}


