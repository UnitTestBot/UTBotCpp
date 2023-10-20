<!---
name: Symbolic Stdin
route: /docs/cpp/advanced/symbolic-stdin
parent: Documentation
menu: Advanced
description: This page provides a detailed description on how UnitTestBot handles input functions such as read, scanf, etc.
--->

# Symbolic `stdin`

UnitTestBot is able to generate tests for C code that reads values from `stdin`, a _file descriptor_ or 
`STDIN_FILENO`.
UnitTestBot marks read values as symbolic, generates tests, puts the necessary data into a C string buffer and 
redirects `stdin` to the buffer so the tests can be executed properly.

<details>
<summary> Here is an example of a function that reads values from `stdin`:</summary>

###### [Source code example](https://github.com/UnitTestBot/UTBotCpp/blob/main/integration-tests/c-example/lib/symbolic_stdin.c)

```cpp
int check_password(int fd) {
  char buf[5];
  if (read(fd, buf, 5) != -1) {
    if (buf[0] == 'h' && buf[1] == 'e' &&
	buf[2] == 'l' && buf[3] == 'l' &&
	buf[4] == 'o')
      return 1;
  }
  return 0;
}
```

###### Redirecting `stdin`

```cpp
void utbot_redirect_stdin(const char* buf, int &res) {
    int fds[2];
    if (pipe(fds) == -1) {
        res = -1;
        return;
    }
    close(STDIN_FILENO);
    dup2(fds[0], STDIN_FILENO);
    write(fds[1], buf, 64);
    close(fds[1]);
}
```

###### Test code example

```cpp
TEST(regression, check_password_test_1)
{
    // Redirect stdin
    char stdin_buf[] = "hello";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }
    // Construct input
    int fd = 0;

    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = check_password(fd);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, check_password_test_2)
{
    // Construct input
    int fd = 1;

    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = check_password(fd);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, check_password_test_3)
{
    // Construct input
    int fd = 3;

    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = check_password(fd);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, check_password_test_4)
{
    // Redirect stdin
    char stdin_buf[] = "\x97""\x97""\x97""\x97""\x97""";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }
    // Construct input
    int fd = 0;

    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = check_password(fd);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, check_password_test_5)
{
    // Redirect stdin
    char stdin_buf[] = "he\0""hh";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }
    // Construct input
    int fd = 0;

    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = check_password(fd);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, check_password_test_6)
{
    // Redirect stdin
    char stdin_buf[] = "hell\0""";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }
    // Construct input
    int fd = 0;

    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = check_password(fd);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, check_password_test_7)
{
    // Construct input
    int fd = 1024;

    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = check_password(fd);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, check_password_test_8)
{
    // Redirect stdin
    char stdin_buf[] = "h\0""hhh";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }
    // Construct input
    int fd = 0;

    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = check_password(fd);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, check_password_test_9)
{
    // Construct input
    int fd = -1;

    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = check_password(fd);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, check_password_test_10)
{
    // Redirect stdin
    char stdin_buf[] = "hel\0""h";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }
    // Construct input
    int fd = 0;

    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = check_password(fd);

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

### Trouble with interactive mode

In the previous version, UnitTestBot didn't preprocess functions that were using `stdin` before sending the bitcode to 
KLEE. Having added the interactive mode, we faced difficulties: KLEE couldn't work with multiple entry points 
that used `stdin` in one launch. The fact is that KLEE substitutes the original entry point to POSIX wrapper, which 
initializes environment and adds the `stdin`, `stdin_stat`, `stdin_read`, and `model_version` symbolic variables. 
Then KLEE launches this wrapper as if the wrapper was the initial entry point.

UnitTestBot doesn't use these KLEE wrappers now. Instead, UnitTestBot creates the POSIX wrapper for every 
function in `.c` files it generates tests for.

Example:
```cpp
int klee_entry__foo(int utbot_argc, char ** utbot_argv, char ** utbot_envp) {
    klee_init_env(&utbot_argc, &utbot_argv);
    int utbot_result = klee_entry__foo__wrapped(utbot_argc, utbot_argv, utbot_envp);
    check_stdin_read();
    return utbot_result;
}
```

### Files

UnitTestBot now generates tests for functions that have arguments of a `FILE *` type. We take advantage of KLEE 
as it has already been working with `FILE *`.

Firstly, we open files named `A`, `B`, `C`, etc. in a wrapped function that we send to KLEE (the files are named
according to the KLEE restriction).

Example:
```cpp
int klee_entry__lib_input_output_file_file_fgetc__wrapped(int utbot_argc, char ** utbot_argv, char ** utbot_envp) {
    struct _IO_FILE * fA = fopen("A", "r");
    struct _IO_FILE * fB = fopen("B", "r");
    struct _IO_FILE * fC = fopen("C", "r");
    int utbot_result;
    klee_make_symbolic(&utbot_result, sizeof(utbot_result), "utbot_result");
    int utbot_tmp = file_fgetc(fA, fB, fC);
    klee_assume(utbot_tmp == utbot_result);
    return 0;
}
```

As soon as KLEE finishes its work, UnitTestBot writes the KLEE-generated content to a corresponding file and 
generates tests.

Example:
```cpp
TEST(regression, file_fgetc_test_1)
{
    write_to_file("../../../tests/lib/input_output/A", "\0""\0""");
    write_to_file("../../../tests/lib/input_output/C", "p");

    struct _IO_FILE * fA = (UTBot::FILE *) fopen("../../../tests/lib/input_output/A", "r");
    struct _IO_FILE * fB = (UTBot::FILE *) fopen("../../../tests/lib/input_output/B", "r");
    struct _IO_FILE * fC = (UTBot::FILE *) fopen("../../../tests/lib/input_output/C", "r");

    int actual = file_fgetc(fA, fB, fC);
    EXPECT_EQ(4, actual);
}
```
