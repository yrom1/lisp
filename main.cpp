#include <bits/stdc++.h>

#include "print.h"

template <typename T>
struct Data {
  typedef T value_type;
  T value;
  explicit Data(const T& x) : value(x) {}
  explicit operator T() const { return value; }
  template <typename U>
  Data(const Data<U>& x) : value(x.value) {}

  // Semiregular:
  Data(const Data& x) : value(x.value) {}
  Data() {}
  ~Data() {}
  Data& operator=(const Data& x) {
    value = x.value;
    return *this;
  }
  // Regular
  friend bool operator==(const Data& x, const Data& y) {
    return x.value == y.value;
  }
  friend bool operator!=(const Data& x, const Data& y) { return !(x == y); }
  // TotallyOrdered
  friend bool operator<(const Data& x, const Data& y) {
    return x.value < y.value;
  }
  friend bool operator>(const Data& x, const Data& y) { return y < x; }
  friend bool operator<=(const Data& x, const Data& y) { return !(y < x); }
  friend bool operator>=(const Data& x, const Data& y) { return !(x < y); }
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

// template <typename T>
typedef struct SyntaxTree {
  Token::Token token;
  std::string data;
  // Data<T> data; // TODO(yrom1): why doesn't this work?
  std::vector<SyntaxTree> children;
} SyntaxTree;

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

/*
number   : /-?[0-9]+/
operator : '+' | '-' | '*' | '/'
expr     : <number> | '(' <operator> <expr>+ ')'
lispy    : /^/ <expr>+ /$/
*/

SyntaxTree make_tree(std::vector<std::pair<Token::Token, char>> input) {
  SyntaxTree tree;
  while (input.size() != 0) {
    auto pair_token_data = input.back();
    print::prn(pair_token_data);
    input.pop_back();
    if (pair_token_data.first == Token::lbracket) {
      continue;
    } else if (pair_token_data.first == Token::rbracket) {
      return tree;
    } else if (pair_token_data.first == Token::function) {
      tree.token = pair_token_data.first;
      tree.data = pair_token_data.second;
      tree.children.push_back(make_tree(input));
    } else if (pair_token_data.first == Token::terminal) {
      tree.token = pair_token_data.first;
      tree.data = pair_token_data.second;
      return tree;
    } else if (pair_token_data.first == Token::nomatch) {
      std::cout << "BROKEN";
      break;
    }
  }
  return tree;
}

int main() {
  std::cout << std::string("lisp> ");
  std::string input;
  std::getline(std::cin, input);
  auto lex_input = lex(input);
  print::prn(lex_input);
  make_tree(lex_input);
  std::cout << input << std::endl;
}
