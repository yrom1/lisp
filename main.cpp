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
  nil,  // REFACTOR should nil just be a terminal?
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
  // LISPS
  if (elem == "(") return Token::lbracket;
  if (elem == ")") return Token::rbracket;
  // FUNCTIONS
  // if (std::regex_search(elem, std::regex("\\w"))) return Token::function;
  if (std::regex_search(elem, std::regex("list"))) return Token::function;
  if (std::regex_search(elem, std::regex("quote"))) return Token::function;
  if (std::regex_search(elem, std::regex("^[\\+|-]$"))) return Token::function;
  // TERMINALS
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

auto rsplit(std::string s, std::string rgx) {
  std::vector<std::string> output;
  std::regex re{rgx};
  std::copy(std::sregex_token_iterator{s.begin(), s.end(), re, -1},
            std::sregex_token_iterator{}, std::back_inserter(output));
  print::prn("before: ", s);
  print::prn("after: ", output);
  return output;
}

auto lex(std::string input) {
  // (+ 1) valid input
  // (+1) not valid input
  auto clean_input = add_spaces_around_brackets(input);
  auto split_clean_input = rsplit(clean_input, R"(\s+)");
  std::vector<std::pair<Token::Token, std::string>> lex_input;
  for (auto token : split_clean_input) {
    print::prn("token", token);
    for (auto& c : token) {
      c = ::tolower(c);
    }
    print::prn("lower token", token);
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
      tree.data = "()";
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
  nil,
  nomatch,
};
}  // namespace Type

namespace Converter {

namespace __detail {

Type::Type parse_underlying_type(SyntaxTree tree) {
  // FIXME(yrom1): this is kinda code repetition from parsing
  //               but it's only for terminals
  if (std::regex_search(tree.data, std::regex("[0-9]*"))) return Type::number;
  if (tree.token == Token::nil) return Type::nil;
  return Type::nomatch;
}

auto get_terminal_to_underlying_converter(SyntaxTree tree) {
  // FIXME(yrom1) this is hopelessly broken in c++
  if (parse_underlying_type(tree) == Type::number)
    return [](std::string input) { return std::stoi(input); };
  // if (parse_underlying_type(tree) == Type::nil)
  //   return [](std::string input) { return input; };
  // // FIXME(yrom1): control reaches end of non-void function
}

template <typename T>
auto string_to_underlying(std::string input, T converter) {
  return converter(input);
}

template <typename T>
auto underlying_to_string(T input) {
  return std::to_string(input);
}

}  // namespace __detail

auto terminal_to_underlying(SyntaxTree tree) {
  assert(tree.token == Token::terminal);
  return __detail::string_to_underlying(
      tree.data, __detail::get_terminal_to_underlying_converter(tree));
}

template <typename T>
SyntaxTree underlying_to_terminal(T input) {
  return {Token::terminal, __detail::underlying_to_string(input), {}};
}

}  // namespace Converter

template <typename T>
SyntaxTree tree_reduce(SyntaxTree tree, T binary_operator) {
  SyntaxTree output = Converter::underlying_to_terminal(binary_operator(
      Converter::terminal_to_underlying(eval(tree.children[0])),
      Converter::terminal_to_underlying(eval(tree.children[1]))));
  if (tree.children.size() - 2 > 0) {
    for (size_t i = 2; i < tree.children.size(); ++i) {
      output = Converter::underlying_to_terminal(binary_operator(
          Converter::terminal_to_underlying(eval(output)),
          Converter::terminal_to_underlying(eval(tree.children[i]))));
    }
  }
  return output;
}

template <typename T, typename U>
SyntaxTree arity_dispatch(SyntaxTree tree, T uniary_op, U binary_op) {
  // TODO(yrom1): this could be more elegant somehow
  //              (-) doesn't have a no-arity dispatch... so...
  if (tree.children.size() == 0 || tree.children.size() == 1) {
    return uniary_op(tree);
  } else {
    return binary_op(tree);
  }
}

SyntaxTree add_unary(SyntaxTree tree) {
  if (tree.children.size() == 0) {
    return {Token::terminal, "0", {}};
  } else {
    return eval(tree.children[0]);
  }
}

SyntaxTree add_binary(SyntaxTree tree) {
  return tree_reduce(tree, std::plus<>());
}

SyntaxTree add(SyntaxTree tree) {
  return arity_dispatch(tree, add_unary, add_binary);
}

SyntaxTree minus_unary(SyntaxTree tree) {
  if (tree.children.size() == 0) {
    return {Token::error, "ERROR: Unary minus needs one child!", {}};
  } else {
    return Converter::underlying_to_terminal(
        -Converter::terminal_to_underlying(eval(tree.children[0])));
  }
}

SyntaxTree minus_binary(SyntaxTree tree) {
  return tree_reduce(tree, std::minus<>());
}

SyntaxTree minus(SyntaxTree tree) {
  return arity_dispatch(tree, minus_unary, minus_binary);
}

SyntaxTree list(SyntaxTree tree) {
  print::pr("for_each start:");
  auto __print = [](SyntaxTree x) { std::cout << " " << x.data << " "; };
  std::for_each(tree.children.begin(), tree.children.end(), __print);
  print::prn();
  std::for_each(tree.children.begin(), tree.children.end(), eval);
  print::pr("for_each end:");
  std::for_each(tree.children.begin(), tree.children.end(), __print);
  print::prn();
  return tree;
}

SyntaxTree quote(SyntaxTree tree) {
  assert(tree.token == Token::function);
  assert(tree.data == "quote");
  assert(tree.children.size() != 0);
  print::prn("in quote, tree.data", tree.data, "tree.children.size",
             tree.children.size());
  if (tree.children.size() == 1) return tree.children[0];
  tree.data = "list";
  return tree;
}

SyntaxTree dispatch(SyntaxTree tree) {
  if (tree.data == "list") return list(tree);
  if (tree.data == "quote") return quote(tree);
  if (tree.data == "+") return add(tree);
  if (tree.data == "-") return minus(tree);
  return {Token::error, "ERROR: Can't match data in function dispatch!", {}};
}

SyntaxTree string_to_tree(std::string input) {
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
  //  assert(tree_size_pair.second == 0);
  return tree_size_pair.first;
}

SyntaxTree read() {
  std::cout << std::string("lisp> ");
  std::string input;
  std::getline(std::cin, input);
  return string_to_tree(input);
}

// TODO(yrom1): code repetition in eval and tree_to_string

SyntaxTree eval(SyntaxTree tree) {
  if (tree.token == Token::nil || tree.token == Token::terminal) {
    return tree;
  } else if (tree.token == Token::function) {
    return dispatch(tree);
  } else {
    return {Token::error, "ERROR: Can't eval tree token!", {}};
  }
}

std::string tree_to_string(SyntaxTree tree) {
  std::string output;
  print::prn(tree.token, tree.data);
  if (tree.token == Token::terminal || tree.token == Token::nil) {
    return output += tree.data;
  } else {
    assert(tree.token == Token::function);  // it shouldn't be anything else
    output += "(";
    output += tree.data;
    // if (tree.children.size() != 0) {
    //   output += " ";
    // }
    for (const auto& i : tree.children) {
      output += " ";
      output += tree_to_string(i);
    }
    output += ")";
  }
  std::cout << '\n';
  print::prn("tree_to_string: ", output);
  return output;
}

void _print(SyntaxTree tree) { std::cout << tree_to_string(tree) << std::endl; }

void print_tree(SyntaxTree tree) {
  // TODO(yrom1) remove where-ever this is called with just print
  _print(tree);
}
std::string eval_string_to_string(std::string input) {
  return tree_to_string(eval(string_to_tree(input)));
}

// TODO(yrom1): list, quote, atom, eq, car, cdr, cons, cond
// see https://jtra.cz/stuff/lisp/sclr/index.html

void run_tests() {
  assert(eval_string_to_string("1") == "1");
  assert(eval_string_to_string("42") == "42");
  assert(eval_string_to_string("()") == "()");
  assert(eval_string_to_string("(list 1)") == "(list 1)");  // no (1) sugar
  assert(eval_string_to_string("(LIST 1)") == "(list 1)");
  assert(eval_string_to_string("(list 1 2)") == "(list 1 2)");
  assert(eval_string_to_string("(quote 1)") == "1");
  assert(eval_string_to_string("(quote (list 1 2))") == "(list 1 2)");
  assert(eval_string_to_string("(+)") == "0");
  assert(eval_string_to_string("(+ 1 2 3 4)") == "10");
  assert(eval_string_to_string("(+ 1 (+ 2))") == "3");
  assert(eval_string_to_string("(+ (+ 1 2) (+ 3 4))") == "10");
  assert(eval_string_to_string("(+ (+ (+ (+ 2))))") == "2");
  assert(eval_string_to_string("(- 1)") == "-1");
  assert(eval_string_to_string("(- 4 2)") == "2");
  assert(eval_string_to_string("(- (- 1))") == "1");
  assert(eval_string_to_string("(+ 1 (- 4 2))") == "3");
  assert(eval_string_to_string("(+ 9999999 1)") == "10000000");
  // assert(eval_string_to_string("(+ 99999999999999999999 1)") ==
  // "100000000000000000000"); // dumps on stoi assert(eval_string_to_string("(+
  // 1 ())") == "???"); // this core dumps, pass error?
}

void repl() {
  while (true) {
    _print(eval(read()));
  }
}

int main() {
  run_tests();
  repl();
}
