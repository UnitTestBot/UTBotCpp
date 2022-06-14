static char **file_list = 0;
static char *stdin_argv[] = { (char *)"-", 0 };

static void set_file_list(char **list) {
    if (!list)
        file_list = stdin_argv;
    else
        file_list = list;
}
