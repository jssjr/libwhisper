#include "whisper.h"
#include "sput.h"

void test_whisper__info(void);

int main(int argc, char *argv[]) {
  sput_start_testing();

  sput_enter_suite("libwhisper: wsp file operations");
  sput_run_test(test_whisper__info);

  sput_finish_testing();
  return sput_get_return_value();
}
