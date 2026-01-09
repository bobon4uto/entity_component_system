#include <bits/endian.h>
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

// shorthands to some nob functions
#define run cmd_run
#define cc_inputs nob_cc_inputs
#define cc_output nob_cc_output

Cmd cmd0 = {0};
Cmd *cmd = &cmd0;

void cc() {
  nob_cc(cmd);
  nob_cc_flags(cmd);
}

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"

#define TEST_FILE "main.c"
#define TEST_EXECUTABLE "main"

#define CR(X)                                                                  \
  ret &= X;                                                                    \
  if (!ret)                                                                    \
    printf("%s:%d: ERROR: " #X " returned false\n", __FILE__, __LINE__);       \
  if (!ret)                                                                    \
  return ret

// functions that return boolean via ret variable
bool bmain(int argc, char **argv);
bool prepare();
bool build();
bool test();
bool clean();

void help(const char *program_name) {
  // help message
  nob_log(INFO,
          "usage: %s [hbtc]\n" //
          "    h - print this help message\n"
          "    b - build the project\n"
          "    t - run tests\n"
          "    c - clean object files\n"
          "NOTE: all commands are executed sequentially, that is,\n"
          "    %s hb will print help, and then build.\n"
          "    if at any point an error occures, terminates.\n",
          program_name, program_name);
}
bool parsec(char c, const char *program_name) {
  bool ret = true;

  switch (c) {
  case 'b':
    CR(build());
    break;
  case 't':
    CR(test());
    break;
  case 'c':
    CR(clean());
    break;
  default:
    help(program_name);
  }

  return ret;
}

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);
  if (!bmain(argc, argv))
    return 1;
  return 0;
}
bool bmain(int argc, char **argv) {
  bool ret = true;
  CR(prepare());

  if (argc > 1) {
    for (size_t i = 0; argv[1][i] != '\0'; i++) {
      CR(parsec(argv[1][i], argv[0]));
    }
  } else {
    // default behaviour
    CR(parsec('c', argv[0]));
    CR(parsec('b', argv[0]));
    CR(parsec('t', argv[0]));
  }

  return ret;
}

bool prepare() {
  bool ret = true;
  CR(mkdir_if_not_exists(BUILD_FOLDER));

  return ret;
}

bool build() {
  bool ret = true;
  cc();
  cc_inputs(cmd, SRC_FOLDER TEST_FILE);
  cc_output(cmd, BUILD_FOLDER TEST_EXECUTABLE);
  CR(run(cmd));

  return ret;
}
bool test() {
  bool ret = true;
  cmd_append(cmd, BUILD_FOLDER TEST_EXECUTABLE);
  run(cmd);
  return ret;
}

bool clean() {
  bool ret = true;
  // if (file_exists())
  //   delete_file(OBJ_FOLDER);

  return ret;
}
