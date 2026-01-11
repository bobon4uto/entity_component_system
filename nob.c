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
  cmd_append(cmd, "-g");
  cmd_append(cmd, "-O0");
}
void pp_only() {
  cmd_append(cmd, "-E");
}

#define BUILD_FOLDER "build/"
#define BUILD_UTIL_FOLDER BUILD_FOLDER "util/"
#define BUILD_TEST_FOLDER BUILD_FOLDER "test/"
#define SRC_FOLDER "src/"
#define UTIL_FOLDER SRC_FOLDER "util/"
#define GEN_FOLDER SRC_FOLDER "gen/"

#define METAM_FILE "metam.c"
#define METAM_EXECUTABLE "metam"

#define TEST_FILE "main.c"
#define TEST_EXECUTABLE "main"

#define GEN_FILE "main.gen.c"

#define CR(X)                                                                  \
  ret &= X;                                                                    \
  if (!ret)                                                                    \
    printf("%s:%d: ERROR: " #X " returned false\n", __FILE__, __LINE__);       \
  if (!ret)                                                                    \
  return ret

// functions that return boolean via ret variable
bool bmain(int argc, char **argv);
bool build_general(const char* in, const char* out);
bool prepare();
bool preprocess();
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
  CR(mkdir_if_not_exists(BUILD_UTIL_FOLDER));
  CR(mkdir_if_not_exists(BUILD_TEST_FOLDER));

  return ret;
}
bool preprocess() {
  bool ret = true;
  //cc();
  //pp_only();
  //cc_inputs(cmd, SRC_FOLDER );
  //cc_output(cmd, GEN_FOLDER GEN_FILE);
  //CR(run(cmd));

  //cmd_append(cmd, "cat", GEN_FOLDER GEN_FILE);
  //CR(run(cmd));

  return ret;
}
bool build_general_no_check(const char* in, const char* out) { 
  bool ret = true;
    cc();
    cc_inputs(cmd, in);
    cc_output(cmd, out);
    CR(run(cmd));
  return ret;
}

bool build_general(const char* in, const char* out) {
  bool ret = true;
  if (nob_needs_rebuild1(out, SRC_FOLDER "vups.h") || nob_needs_rebuild1(out, in)) {
    build_general_no_check(in, out);
  }
  return ret;
}

bool build() {
  bool ret = true;

  build_general(UTIL_FOLDER METAM_FILE, BUILD_UTIL_FOLDER METAM_EXECUTABLE);
  //preprocess();
  build_general(SRC_FOLDER TEST_FILE, BUILD_FOLDER TEST_EXECUTABLE);

  return ret;
}
bool test_metam() {
  bool ret = true;
  nob_log(INFO,"testing metam");
  cmd_append(cmd,BUILD_UTIL_FOLDER METAM_EXECUTABLE, "./src/tests/metam_test_1.c", "-f", "print", "-o", "./src/tests/metam_test_1.c.gen.c");
  run(cmd);
  CR(build_general_no_check("src/tests/metam_test_1.c", "build/test/metam_test_1"));
  cmd_append(cmd,"build/test/metam_test_1");
  CR(run(cmd));
  nob_log(INFO,"test metam_test_1 >>passed<<");
  return ret;
}
bool test() {
  bool ret = true;
  test_metam();


  cmd_append(cmd, BUILD_UTIL_FOLDER METAM_EXECUTABLE, SRC_FOLDER TEST_FILE,"-f", "world_spawn", "-o", GEN_FOLDER GEN_FILE);
  run(cmd);
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
