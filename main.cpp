#include <bits/stdc++.h>

#include "print.h"

struct SyntaxTree;
SyntaxTree eval(SyntaxTree);
void print_tree(SyntaxTree);  // TEMP remove

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
  error,
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
  auto split_clean_input = rsplit(clean_input, R"(\s*)");
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

namespace Type {
// not enum class because I want it convertable to int for debugging
enum Type {
  number,
  nomatch,
};
}  // namespace Type

namespace Converter {

namespace __detail {

Type::Type parse_underlying_type(SyntaxTree tree) {
  // DONE
  // FIXME(yrom1): this is kinda code repetition from parsing
  //               but it's only for terminals
  if (std::regex_search(tree.data, std::regex("[0-9]*"))) return Type::number;
  return Type::nomatch;
}

auto get_terminal_to_underlying_converter(SyntaxTree tree) {
  // DONE
  if (parse_underlying_type(tree) == Type::number)
    return [](std::string input) { return std::stoi(input); };
}

template <typename T>
auto string_to_underlying(std::string input, T converter) {
  // DONE
  return converter(input);
}

template <typename T>
auto underlying_to_string(T input) {
  // DONE
  return std::to_string(input);
}

}  // namespace __detail

auto convert_terminal_to_underlying(SyntaxTree tree) {
  // DONE
  assert(tree.token == Token::terminal);
  return __detail::string_to_underlying(
      tree.data, __detail::get_terminal_to_underlying_converter(tree));
}

template <typename T>
SyntaxTree convert_underlying_to_terminal(T input) {
  // DONE
  return {Token::terminal, __detail::underlying_to_string(input), {}};
}

}  // namespace Converter

template <typename T>
SyntaxTree tree_reduce(SyntaxTree tree, T binary_operator) {
  // DONE
  // TODO(yrom1): I'm pretty sure this can be done in one loop
  auto output = Converter::convert_underlying_to_terminal(binary_operator(
      Converter::convert_terminal_to_underlying(eval(tree.children[0])),
      Converter::convert_terminal_to_underlying(eval(tree.children[1]))));
  if (tree.children.size() - 2 > 0) {
    for (size_t i = 2; i < tree.children.size() - 1; ++i) {
      output += Converter::convert_underlying_to_terminal(
          Converter::convert_terminal_to_underlying(
              binary_operator(eval(tree.children[i])),
              Converter::convert_terminal_to_underlying(
                  eval(tree.children[i + 1]))));
    }
  }
  return output;
}

template <typename T, typename U>
SyntaxTree arity_dispatch(SyntaxTree tree, T uniary_op, U binary_op) {
  // DONE
  // TODO(yrom1): this could be more elegant somehow
  //              (-) doesn't have a no-arity dispatch... so...
  if (tree.children.size() == 0 || tree.children.size() == 1) {
    return uniary_op(tree);
  } else {
    return binary_op(tree);
  }
}

SyntaxTree add_unary(SyntaxTree tree) {
  // DONE
  if (tree.children.size() == 0) {
    return {Token::terminal, "0", {}};
  } else {
    return eval(tree.children[0]);
  }
}

SyntaxTree add_binary(SyntaxTree tree) {
  // DONE
  return tree_reduce(tree, std::plus<>());
}

SyntaxTree add(SyntaxTree tree) {
  // DONE
  return arity_dispatch(tree, add_unary, add_binary);
}

SyntaxTree minus_unary(SyntaxTree tree) {
  // DONE
  if (tree.children.size() == 0) {
    return {Token::error, "ERROR: Unary minus needs one child!", {}};
  } else {
    // FIXME TEMP removing minus for debug
    return Converter::convert_underlying_to_terminal(
        -Converter::convert_terminal_to_underlying(eval(tree.children[0])));
  }
}

SyntaxTree minus_binary(SyntaxTree tree) {
  // DONE
  return tree_reduce(tree, std::minus<>());
}

SyntaxTree minus(SyntaxTree tree) {
  // DONE
  return arity_dispatch(tree, minus_unary, minus_binary);
}

SyntaxTree dispatch(SyntaxTree tree) {
  // DONE
  if (tree.data == "+") {
    return add(tree);
  }
  if (tree.data == "-") {
    return minus(tree);
  }
  return {Token::error, "ERROR: Can't match data in function dispatch!", {}};
}

SyntaxTree string_to_tree(std::string input) {
  // DONE
  // -- Lex --
  auto lex_input = lex(input);
  print::prn(lex_input);
  std::reverse(lex_input.begin(), lex_input.end());
  print::prn(lex_input);

  // -- Parse --
  auto tree_size_pair = make_tree(lex_input);
  print_tree(tree_size_pair.first);
  // if it's not an error happened, because we need to consume the entire
  // lex'ed token vector into a tree
  assert(tree_size_pair.second == 0);
  return tree_size_pair.first;
}

SyntaxTree read() {
  // DONE
  std::cout << std::string("lisp> ");
  std::string input;
  std::getline(std::cin, input);
  return string_to_tree(input);
}
// somewher eelse
// string_to_underlying(tree.data, get_underlying_converter(tree.data));

SyntaxTree eval(SyntaxTree tree) {
  // DONE
  if (tree.token == Token::terminal) {
    return tree;
  } else if (tree.token == Token::function) {
    return dispatch(tree);
  } else {
    return {Token::error, "ERROR: Can't eval tree token!", {}};
  }
}

std::string tree_to_string(SyntaxTree tree) {
  // DONE... probably wrong
  std::string output;
  print::pr(tree.token, tree.data);
  if (tree.token == Token::terminal) {
    return output += tree.data;
  } else {
    assert(tree.token == Token::function);  // it shouldn't be anything else
    output += " (";
    output += tree.data;
    output += " ";
    std::cout << ('(');
    for (const auto& i : tree.children) {
      output += tree_to_string(i);
    }
    output += ") ";
    std::cout << (')');
  }
  std::cout << '\n';
  print::prn(output);
  return output;
}

void _print(SyntaxTree tree) {
  // DONE
  std::cout << tree_to_string(tree) << std::endl;
}

void print_tree(SyntaxTree tree) {
  // DONE
  // TODO(yrom1) remove where-ever this is called with just print
  _print(tree);
}
std::string eval_string_to_string(std::string input) {
  // DONE
  return tree_to_string(eval(string_to_tree(input)));
}

void run_tests() {
  // DONE
  assert(eval_string_to_string("1") == "1");
  // assert(eval_string("()") == ???); // FIXME
  assert(eval_string_to_string("(+)") == "0");
  assert(eval_string_to_string("(+ 1 2 3 4)") == "10");
  assert(eval_string_to_string("(+ 1 (+ 2))") == "3");
  assert(eval_string_to_string("(+ (+ 1 2) (+ 3 4))") == "10");
  assert(eval_string_to_string("(+ (+ (+ (+ 2))))") == "2");
  assert(eval_string_to_string("(- 1)") == "-1");
  assert(eval_string_to_string("(- 4 2)") == "2");
  assert(eval_string_to_string("(- (- 1))") == "1");
  assert(eval_string_to_string("(+ 1 (- 4 2))") == "3");
}

void repl() {
  // DONE
  while (true) {
    _print(eval(read()));
  }
}

int main() {
  run_tests();
  repl();
}
