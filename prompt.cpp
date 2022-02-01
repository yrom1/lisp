#include <bits/stdc++.h>
#include <format>

int main(int argc, char** argv) {
  std::cout << "lisp 0.0.1\n";
  while (true) {
    std::cout << "lisp> ";
    std::string input;
    std::cin >> input;
    std::format("No you're a {}!\n", input);
  }
}
