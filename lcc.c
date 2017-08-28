#include <stdio.h>
#include <assert.h>

int main(void) {
  char *s1 = "1234567";

  char *s2 = string(s1);
  char *s3 = string(s2);
  assert (s2 == s3);
  return 0;
}
