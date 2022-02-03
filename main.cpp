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
  nil,
  nomatch,
};
}  // namespace Token

// template <typename T>
struct SyntaxTree {
  Token::Token token;
  std::string data;
  // Data<T> data; // TODO(yrom1): why doesn't this work?
  std::vector<SyntaxTree> children;
};

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
  // TODO(yrom1): switch from char to std::string
  //              (+ 42) -> 6 right now lol
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

    // 1 -> 1
    if (pair_token_data.first == Token::terminal) {
      print::prn("terminal");
      tree.token = pair_token_data.first;
      tree.data = pair_token_data.second;
      break;
    }

    // An S-expr MUST start with a lbracket!
    if (pair_token_data.first != Token::lbracket) {
      print::prn(input.size(), "ERROR: Unbalanced lbracket!",
                 "next: ", input.back().first, input.back().second,
                 "size: ", input.size());
      break;
    }

    // We're in an S-expr, can be either F, rbracket
    print::prn("input.size", input.size());
    auto sexpr_pair_token_data = input.back();
    input.pop_back();

    // Empty () pair -> NIL
    if (sexpr_pair_token_data.first == Token::rbracket) {
      tree.token = Token::nil;
      tree.data = "NIL";
      break;
    }

    if (sexpr_pair_token_data.first == Token::function) {
      print::prn("function");
      tree.token = sexpr_pair_token_data.first;
      tree.data = sexpr_pair_token_data.second;

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
          auto child_pair_tree_size = make_tree(input);
          tree.children.push_back(child_pair_tree_size.first);
          while (input.size() != child_pair_tree_size.second) {
            input.pop_back();
          }
        }
      }

      print::prn("MUST BE ')': ", input.back().second);
      assert(input.back().second == ')');
      input.pop_back();
      break;
    }
    print::prn("ending loop");
  }
  return std::make_pair(tree, input.size());
}

int eval(SyntaxTree tree);

template <typename T>
int tree_reduce(SyntaxTree tree, T binary_operator) {
  int total = 0;
  while (tree.children.size() != 0) {
    // children can either be F or T
    if (tree.children.back().token == Token::terminal) {
      total = binary_operator(total, std::stoi(tree.children.back().data));
      tree.children.pop_back();
    } else if (tree.children.back().token == Token::function) {
      total = binary_operator(total, eval(tree.children.back()));
      tree.children.pop_back();
    } else {
      print::prn("ERROR: Can't eval token in '+' function!");
      return -42;
    }
  }
  return total;
}

int add_function(SyntaxTree tree) { return tree_reduce(tree, std::plus<>()); }

int minus_function(SyntaxTree tree) {
  return tree_reduce(tree, std::minus<>());
}

int eval(SyntaxTree tree) {
  if (tree.token == Token::terminal) {
    return std::stoi(tree.data);
  } else if (tree.token == Token::function) {
    // TODO(yrom1) implement functions
    // just '+' for now hard coded
    // FIXME total can overflow
    // (+) -> 0 in sbcl
    if (tree.data == "+") {
      return add_function(tree);
    } else if (tree.data == "-") {
      return minus_function(tree);
    } else {
      print::prn("ERROR: Can't recognize function!");
      return -42;  // TEMP
    }
  } else {
    print::prn("ERROR: Can't eval tree token!");
    return -42;  // TEMP
  }
}

auto eval_string(std::string input) {
  auto lex_input = lex(input);
  print::prn(lex_input);
  std::reverse(lex_input.begin(), lex_input.end());
  print::prn(lex_input);

  auto tree_size_pair = make_tree(lex_input);
  print_tree(tree_size_pair.first);
  // if it's not error happened
  assert(tree_size_pair.second == 0);
  auto output = eval(tree_size_pair.first);
  print::prn();
  print::prn(input, "->", output);
  return output;
}

void repl() {
  while (true) {
    std::cout << std::string("lisp> ");
    std::string input;
    std::getline(std::cin, input);
    print::prn(eval_string(input));
  }
}

void run_tests() {
  assert(eval_string("1") == 1);
  // assert(eval_string("()") == ???); // FIXME
  assert(eval_string("(+ 1 2 3 4)") == 10);
  assert(eval_string("(+ 1 (+ 2))") == 3);
  assert(eval_string("(+ (+ 1 2) (+ 3 4))") == 10);
  assert(eval_string("(+ (+ (+ (+ 2))))") == 2);
}

int main() {
  run_tests();
  repl();
}
