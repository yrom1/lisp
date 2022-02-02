#include <bits/stdc++.h>

#include "print.h"

template <typename T>
// T is Semiregular
struct Node {
  typedef T value_type;
  T value;

  T operator()() { return value; }

  explicit Node(const T& x) : value(x) {}
  Node(const Node& x) : value(x.value) {}
  Node() {}
  ~Node() {}
  Node& operator=(const Node& x) {
    value = x.value;
    return *this;
  }
};

namespace Token {
// needs to be printable ie convertable
enum Token {
  /*
  '('     : lbracket
  ')'     : rbracket
  '[0-9]' : terminal
  */
  lbracket,
  rbracket,
  function,
  terminal,
  nomatch,
};
}  // namespace Token

auto parse_elem(char input) {
  std::string elem = {input};
  if (elem == "(") return Token::lbracket;
  if (elem == ")") return Token::rbracket;
  if (std::regex_search(elem, std::regex("\\+|-"))) return Token::function;
  if (std::regex_search(elem, std::regex("[0-9]"))) return Token::terminal;
  return Token::nomatch;
}

auto lex(std::string input) {
  std::vector<std::pair<Token::Token, char>> lex_input;
  std::string current;
  for (auto elem : input) {
    if (elem == ' ') {
      continue;
    }
    auto elem_token = parse_elem(elem);
    lex_input.push_back(std::make_pair(elem_token, elem));
  }
  return lex_input;
}

template <typename T, typename U>
auto make_tree(std::vector<std::pair<T, U>> input) {}

int main() {
  std::cout << std::string("lisp> ");
  std::string input;
  std::getline(std::cin, input);
  auto lex_input = lex(input);
  print::prn(lex_input);
  std::cout << input << std::endl;
}
