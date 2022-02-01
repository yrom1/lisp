#define FMT_HEADER_ONLY
#include <bits/stdc++.h>

#include "/home/ryan/Downloads/fmt/include/fmt/format.h"

int main() {
  fmt::print("lisp version 0.0.1\n");
  fmt::print("ctrl + c to exit\n\n");
  while (true) {
    fmt::print("lisp> ");
    std::string input;
    std::getline(std::cin, input);
    fmt::print("you said {}!\n", input);
  }
}
