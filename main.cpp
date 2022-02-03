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

void print_tree(const SyntaxTree& input) {
  print::pr(input.token, input.data);
  if (input.token == Token::terminal) {
    return;
  } else {
    for (const auto& i : input.children) {
      print::pr('[');
      print_tree(i);
      print::pr(']');
    }
  }
  print::prn();
}
//  std::vector<Token::Token>>::size
std::pair<SyntaxTree, std::size_t> make_tree(
    std::vector<std::pair<Token::Token, char>> input) {
  SyntaxTree tree;
  while (input.size() != 0) {
    print::prn("input.size", input.size(), input.back().second);
    auto pair_token_data = input.back();
    input.pop_back();

    // must start with lbracket
    if (pair_token_data.first != Token::lbracket) {
      print::prn(input.size(), "ERROR: Unbalanced lbracket!");
      break;
    }

    if (pair_token_data.first == Token::terminal) {
      print::prn("terminal");
      tree.token = pair_token_data.first;
      tree.data = pair_token_data.second;
      break;
    }

    if (pair_token_data.first == Token::function) {
      print::prn("function");
      tree.token = pair_token_data.first;
      tree.data = pair_token_data.second;

      // pop function
      input.pop_back();
      while (input.back().first != Token::rbracket) {
        print::prn("child");
        // we cant hit a function, doesn't make sense, only T, '(', or ')'
        if (input.back().first == Token::terminal) {
          SyntaxTree terminal = {input.back().first,
                                 std::string{input.back().second},
                                 std::vector<SyntaxTree>{}};
          tree.children.push_back(terminal);
          input.pop_back();
        } else if (input.back().first == Token::lbracket) {
          auto pair_tree_size = make_tree(input);
          tree.children.push_back(pair_tree_size.first);
          while (input.size() != pair_tree_size.second) {
            input.pop_back();
          }
        } else if (input.back().first == Token::rbracket) {
          input.pop_back();
          break;
        }
      }

      // pop rbracket
      input.pop_back();
      break;
    }
  }
  return std::make_pair(tree, input.size());
}

int main() {
  std::cout << std::string("lisp> ");
  std::string input;
  std::getline(std::cin, input);
  auto lex_input = lex(input);
  print::prn(lex_input);
  std::reverse(lex_input.begin(), lex_input.end());
  print::prn(lex_input);
  auto tree_size_pair = make_tree(lex_input);
  print_tree(tree_size_pair.first);
  std::cout << input << std::endl;
}
