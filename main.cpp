#include <bits/stdc++.h>

#include "print.h"

struct SyntaxTree;
int eval(SyntaxTree tree);

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

auto parse_elem(std::string elem) {
  print::prn("parse:", elem);
  if (elem == "(") return Token::lbracket;
  if (elem == ")") return Token::rbracket;
  if (std::regex_search(elem, std::regex("^[\\+|-]$"))) return Token::function;
  if (std::regex_search(elem, std::regex("[0-9]*"))) return Token::terminal;
  return Token::nomatch;
}

auto add_spaces_around_brackets(std::string input) {
  std::string clean_input;
  print::prn(input, "->", clean_input);
  for (auto elem : input) {
    if (elem == '(' || elem == ')') {
      // FIXME if in the future I implement strings this needs fixing
      // TODO(yrom1) Please tell me I can do this in one line
      std::string elem_str(1, elem);
      std::string space(" ");
      clean_input += space + elem_str + space;
    } else {
      clean_input += elem;
    }
  }
  print::prn(input, "->", clean_input);
  return clean_input;
}

std::vector<std::string> rsplit(const std::string& s,
                                const std::string& regular_expression) {
  const std::regex rgx(regular_expression);
  std::sregex_token_iterator iter(s.begin(), s.end(), rgx, -1);
  std::vector<std::string> output;
  for (std::sregex_token_iterator end; iter != end; ++iter) {
    output.push_back(iter->str());
  }
  return output;
}

auto lex(std::string input) {
  // (+ 1) valid input
  // (+1) not valid input
  auto clean_input = add_spaces_around_brackets(input);
  auto split_clean_input = rsplit(clean_input, R"(\s)");
  std::vector<std::pair<Token::Token, std::string>> lex_input;
  for (auto token : split_clean_input) {
    if (token.size() > 0) {
      lex_input.push_back(
          std::make_pair(parse_elem(std::string(token)), token));
    }
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
    std::vector<std::pair<Token::Token, std::string>> input) {
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
      assert(input.back().second == ")");
      input.pop_back();
      break;
    }
    print::prn("ending loop");
  }
  return std::make_pair(tree, input.size());
}

template <typename T>
auto tree_reduce(SyntaxTree tree, T binary_operator) {
  auto total = binary_operator(eval(tree.children[0]), eval(tree.children[1]));
  if (tree.children.size() - 2 > 0) {
    for (size_t i = 2; i < tree.children.size() - 1; ++i) {
      total +=
          binary_operator(eval(tree.children[i]), eval(tree.children[i + 1]));
    }
  }
  return total;
}

template <typename T, typename U>
auto arity_dispatch(SyntaxTree tree, T uniary_op, U binary_op) {
  // TODO(yrom1): this could be more elegant somehow
  //              (-) doesn't have a no-arity dispatch... so...
  if (tree.children.size() == 0 || tree.children.size() == 1) {
    return uniary_op(tree);
  } else {
    return binary_op(tree);
  }
}

auto add_unary(SyntaxTree tree) {
  if (tree.children.size() == 0) {
    return 0;
  } else {
    return eval(tree.children[0]);
  }
}

auto add_binary(SyntaxTree tree) { return tree_reduce(tree, std::plus<>()); }

auto add(SyntaxTree tree) {
  return arity_dispatch(tree, add_unary, add_binary);
}

auto minus_unary(SyntaxTree tree) {
  if (tree.children.size() == 0) {
    print::prn("ERROR: Unary minus needs one child!");
    return -42;  // TEMP
  } else {
    return -eval(tree.children[0]);
  }
}

auto minus_binary(SyntaxTree tree) { return tree_reduce(tree, std::minus<>()); }

auto minus(SyntaxTree tree) {
  return arity_dispatch(tree, minus_unary, minus_binary);
}

auto dispatch(SyntaxTree tree) {
  if (tree.data == "+") {
    return add(tree);
  }
  if (tree.data == "-") {
    return minus(tree);
  }
  return -42;  // TEMP
}

int eval(SyntaxTree tree) {
  if (tree.token == Token::terminal) {
    return std::stoi(tree.data);
  } else if (tree.token == Token::function) {
    return dispatch(tree);
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
  assert(eval_string("(+)") == 0);
  assert(eval_string("(+ 1 2 3 4)") == 10);
  assert(eval_string("(+ 1 (+ 2))") == 3);
  assert(eval_string("(+ (+ 1 2) (+ 3 4))") == 10);
  assert(eval_string("(+ (+ (+ (+ 2))))") == 2);
  assert(eval_string("(- 1)") == -1);
  assert(eval_string("(- 4 2)") == 2);
  assert(eval_string("(- (- 1))") == 1);
  assert(eval_string("(+ 1 (- 4 2))") == 3);
}

int main() {
  run_tests();
  repl();
}
