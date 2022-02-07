#include <bits/stdc++.h>

#include "print.h"

struct SyntaxTree;
SyntaxTree eval(SyntaxTree);
std::string tree_to_string(SyntaxTree);
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
  nil,
  lbracket,
  rbracket,
  t,
  function,
  terminal,
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

auto add_spaces_around_brackets(std::string input) -> std::string {
  print::prn("calling add spaces");
  std::string clean_input;
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

auto rsplit(std::string s, std::string rgx) -> std::vector<std::string> {
  print::prn("calling rsplit");
  std::vector<std::string> output;
  std::regex re{rgx};
  std::copy(std::sregex_token_iterator{s.begin(), s.end(), re, -1},
            std::sregex_token_iterator{}, std::back_inserter(output));
  print::prn("after: ", output);
  return output;
}

auto parse_elem(std::string elem) -> Token::Token {
  print::prn("calling parse elem");
  if (std::regex_search(elem, std::regex("\\(\\s*\\)"))) return Token::nil;
  if (elem == "(") return Token::lbracket;
  if (elem == ")") return Token::rbracket;
  if (elem == "t") return Token::t;
  if (std::regex_search(elem, std::regex("[A-Za-z_]+|-|\\+"))) return Token::function;
  if (std::regex_search(elem, std::regex("[0-9]+"))) return Token::terminal;
  throw std::logic_error("ERROR: Can't parse element to token!");
}

auto lex(std::string input) -> std::vector<std::pair<Token::Token, std::string>> {
  print::prn("calling lex");
  auto clean_input = add_spaces_around_brackets(input);
  auto split_clean_input = rsplit(clean_input, R"(\s+)");
  std::vector<std::pair<Token::Token, std::string>> lex_input;
  for (auto token : split_clean_input) {
    for (auto& c : token) {
      c = ::tolower(c);
    }
    if (token.size() > 0) {
      lex_input.push_back(
          std::make_pair(parse_elem(std::string(token)), token));
    }
  }
  return lex_input;
}

template <typename T>
auto return_pop_back(std::vector<T>& input) {
  auto back = input.back();
  input.pop_back();
  return back;
}

auto make_tree(std::vector<std::pair<Token::Token, std::string>> input) -> std::pair<SyntaxTree, std::size_t>  {
  print::prn("calling make tree");
  SyntaxTree tree;
  while (input.size() != 0) {
    auto pair_token_data = return_pop_back(input);
    if (pair_token_data.first == Token::terminal
     || pair_token_data.first == Token::t
     || pair_token_data.first == Token::nil) {
      tree.token = pair_token_data.first;
      tree.data = pair_token_data.second;
    } else {
      assert(pair_token_data.first == Token::lbracket);
      pair_token_data = return_pop_back(input);

      assert(pair_token_data.first == Token::function);
      tree.token = pair_token_data.first;
      tree.data = pair_token_data.second;

      assert(input.size() != 0);
      while (input.back().first != Token::rbracket) {
        auto pair_tree_size = make_tree(input);
        tree.children.push_back(pair_tree_size.first);
        input.resize(pair_tree_size.second);
        assert(input.size() != 0);
      }
      assert(input.back().first == Token::rbracket);
      input.pop_back();
    }
    break;
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

auto parse_underlying_type(SyntaxTree tree) -> Type::Type {
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
auto underlying_to_terminal(T input) -> SyntaxTree {
  return {Token::terminal, __detail::underlying_to_string(input), {}};
}

}  // namespace Converter

template <typename T>
auto tree_reduce(SyntaxTree tree, T binary_operator) -> SyntaxTree {
  SyntaxTree output = Converter::underlying_to_terminal(binary_operator(
      Converter::terminal_to_underlying(eval(tree.children[0])),
      Converter::terminal_to_underlying(eval(tree.children[1]))));
  // FIXME(yrom1): siplify to only one if, maybe while(!nil) {...}
  if (tree.children.size() - 2 > 0) {
    for (size_t i = 2; i < tree.children.size(); ++i) {
      if (tree.data != "()") {  // FIXME(yrom1): what about Token::t ?
        output = Converter::underlying_to_terminal(binary_operator(
            Converter::terminal_to_underlying(eval(output)),
            Converter::terminal_to_underlying(eval(tree.children[i]))));
      }
    }
  }
  return output;
}

template <typename T, typename U>
auto arity_dispatch(SyntaxTree tree, T uniary_op, U binary_op) -> SyntaxTree {
  // TODO(yrom1): this could be more elegant somehow
  //              (-) doesn't have a no-arity dispatch... so...
  if (tree.children.size() == 0 || tree.children.size() == 1) {
    return uniary_op(tree);
  } else {
    return binary_op(tree);
  }
}

auto add_unary(SyntaxTree tree) -> SyntaxTree {
  if (tree.children.size() == 0) {
    return {Token::terminal, "0", {}};
  } else {
    return eval(tree.children[0]);
  }
}

auto add_binary(SyntaxTree tree) -> SyntaxTree {
  return tree_reduce(tree, std::plus<>());
}

auto add(SyntaxTree tree) -> SyntaxTree {
  return arity_dispatch(tree, add_unary, add_binary);
}

auto minus_unary(SyntaxTree tree) -> SyntaxTree{
  if (tree.children.size() == 0) {
    return {Token::error, "ERROR: Unary minus needs one child!", {}};
  } else {
    return Converter::underlying_to_terminal(
        -Converter::terminal_to_underlying(eval(tree.children[0])));
  }
}

auto minus_binary(SyntaxTree tree) -> SyntaxTree {
  return tree_reduce(tree, std::minus<>());
}

auto minus(SyntaxTree tree) -> SyntaxTree {
  return arity_dispatch(tree, minus_unary, minus_binary);
}

auto list(SyntaxTree tree) -> SyntaxTree {
  print::prn("calling list");
  if (tree.children.size() == 0) {
    return {Token::nil, "()", {}};
  }
  print::pr("for_each start:");
  auto __print = [](SyntaxTree x) { std::cout << " " << x.data << ", "; };
  std::for_each(tree.children.begin(), tree.children.end(), __print);
  print::prn();
  // std::for_each(tree.children.begin(), tree.children.end(), eval);
  for (auto& child : tree.children) {
    print::prn("before: ", child.data);
    child = eval(child);
    print::prn("after: ", child.data);
  }
  tree.children.push_back({Token::nil, "()", {}});
  print::pr("for_each end:");
  std::for_each(tree.children.begin(), tree.children.end(), __print);
  print::prn();
  return tree;
}

auto quote(SyntaxTree tree) -> SyntaxTree {
  assert(tree.token == Token::function);
  assert(tree.data == "quote");
  assert(tree.children.size() != 0);
  print::prn("in quote, tree.data", tree.data, "tree.children.size",
             tree.children.size());
  if (tree.children.size() == 1) return tree.children[0];
  tree.data = "list";
  return tree;
}

auto car(SyntaxTree tree) -> SyntaxTree {
  print::prn("calling car");
  assert(tree.token == Token::function);
  assert(tree.data == "car");
  assert(tree.children.size() > 0);
  assert(tree.children[0].data == "list");
  print::prn("tree.children[0].children.size()", tree.children.size(),
             "tree.children[0].children.size()",
             tree.children[0].children.size());
  return eval(tree.children[0].children[0]);
}

auto cdr(SyntaxTree tree) -> SyntaxTree {
  print::prn("calling cdr");
  // assert(tree.token == Token::function);
  // assert(tree.data == "cdr");
  // assert(tree.children.size() > 0);
  if (tree.children[0].token == Token::nil) {
    return tree.children[0];
    // is this <= check needed? is (list) transformed to () before this
    // function?
  } else if (tree.children[0].data == "list" &&
             tree.children[0].children.size() <= 1) {
    return {Token::nil, "()", {}};
  } else {
    assert(tree.children[0].data == "list");
    // tree.children[0].children[1::]
    tree.children[0].children = std::vector<SyntaxTree>(
        tree.children[0].children.begin() + 1, tree.children[0].children.end());
    return eval(tree.children[0]);
  }
}

auto eq(SyntaxTree tree) -> SyntaxTree {
  assert(tree.token == Token::function);
  assert(tree.data == "eq");
  assert(tree.children.size() == 2);
  // FIXME(yrom1): need to recheck after symbols are added
  //               is (eq 3 3) -> t?
  if (&tree.children[0] == &tree.children[1] ||
      tree_to_string(tree.children[0]) == tree_to_string(tree.children[1])) {
    tree.token = Token::t;
    tree.data = "t";
  } else {
    tree.token = Token::nil;
    tree.data = "()";
  }
  tree.children.clear();
  return tree;
}

auto cons(SyntaxTree tree) -> SyntaxTree {
  assert(tree.token == Token::function);
  assert(tree.data == "cons");
  assert(tree.children.size() == 2);
  return tree;
}

auto atom(SyntaxTree tree) -> SyntaxTree {
  // true if not a cons
  // in my implementation, true if not list, or not cons
  // nil is a list, but not a cons so (atom ()) -> t
  assert(tree.token == Token::function);
  assert(tree.data == "atom");
  assert(tree.children.size() == 1);
  // TODO(yrom1): collapse first and third if...else into one
  if (tree.children[0].token == Token::nil) {
    return {Token::t, "t", {}};
  } else if (tree.children[0].token == Token::function &&
             (tree.children[0].data == "list" ||
              tree.children[0].data == "cons")) {
    return {Token::nil, "()", {}};
  } else {
    return {Token::t, "t", {}};
  }
}

auto null(SyntaxTree tree) -> SyntaxTree {
  assert(tree.token == Token::function);
  assert(tree.data == "null");
  assert(tree.children.size() == 1);
  print::prn("calling null");
  if (tree.children[0].token == Token::nil ||
      eval(tree.children[0]).token == Token::nil) {
    print::prn("found nil, returning t");
    return {Token::t, "t", {}};
  } else {
    print::prn("returning nil");
    return {Token::nil, "()", {}};
  }
}

auto _not(SyntaxTree tree) -> SyntaxTree {
  assert(tree.token == Token::function);
  assert(tree.data == "not");
  tree.data = "null";  // REFACTOR(yrom1): this is only for the assert, should I
                       // do this?
  return null(tree);
}

auto cond(SyntaxTree tree) -> SyntaxTree {
  //            cond
  //              |
  // optional[list (list L_0) ... (list L_n)]
  //
  //   ____________ L ___________
  //   |        ... | ...       |
  // arg0    optional[arg...]  optional[argn]
  //
  // where `n` is the `len(L)`
  // `optional[argn]` only if (`arg0` != Token::function), else `argn`
  //
  // --- cond ---
  // if no children -> `nil`
  // otherwise eval each `L` and return the first non-nil result
  // if all eval to `nil` -> `nil`
  //
  // --- L ---
  // `arg0` is a function or `t` or `nil`
  // `optional[arg...]` are arguments to `arg0` if it is a function
  //     you are allowed to pass arguments that are unused
  // `argn` is the result if `(not (null (eval(arg0)))) -> t`
  // it is optional[argn] as
  assert(tree.token == Token::function);
  assert(tree.data == "cond");
  print::prn("calling cond");
  // if (tree.children.size() == 0) {
  //   return {Token::nil, "()", {}};
  // } else {
  //   for (auto child : tree.children) {
  //     assert(child.children[0].token == Token::function ||
  //            child.children[0].token == Token::nil ||
  //            child.children[0].token == Token::t);
  //     if (child.children[0].token == Token::t) {
  //       return child.children.back();
  //     } else if (child.children[0].token == Token::nil) {
  //       continue;
  //     } else {
  //       assert(child.children[0].token == Token::function);
  //       SyntaxTree sublist = {child.children[0].token,
  //                             child.children[0].data,
  //                             {std::vector(child.children.begin() + 1,
  //                                          child.children.end() - 1)}};
  //       if (_not(null(eval(sublist))).token == Token::t) {
  //         return child.children.back();
  //       }
  //     }
  //   }
  // }
  return {Token::nil, "()", {}};
}

auto dispatch(SyntaxTree tree) -> SyntaxTree {
  if (tree.data == "null") return null(tree);
  if (tree.data == "not") return _not(tree);
  if (tree.data == "cond") return cond(tree);
  if (tree.data == "atom") return atom(tree);
  if (tree.data == "cons") return cons(tree);
  if (tree.data == "eq") return eq(tree);
  if (tree.data == "car") return car(tree);
  if (tree.data == "cdr") return cdr(tree);
  if (tree.data == "list") return list(tree);
  if (tree.data == "quote") return quote(tree);
  if (tree.data == "+") return add(tree);
  if (tree.data == "-") return minus(tree);
  return {Token::error, "ERROR: Can't match data in function dispatch!", {}};
}

auto string_to_tree(std::string input) -> SyntaxTree {
  // -- Lex --
  auto lex_input = lex(input);
  print::prn(lex_input);
  std::reverse(lex_input.begin(), lex_input.end());
  print::prn(lex_input);

  // -- Parse --
  print::prn("----0");
  auto tree_size_pair = make_tree(lex_input);
  print::prn("----1");
  print_tree(tree_size_pair.first);
  // if it's not an error happened, because we need to consume the entire
  // lex'ed token vector into a tree
  //  assert(tree_size_pair.second == 0);
  return tree_size_pair.first;
}

auto read() -> SyntaxTree {
  std::cout << std::string("lisp> ");
  std::string input;
  std::getline(std::cin, input);
  return string_to_tree(input);
}

// TODO(yrom1): code repetition in eval and tree_to_string

auto eval(SyntaxTree tree) -> SyntaxTree {
  if (tree.token == Token::nil || tree.token == Token::terminal ||
      tree.token == Token::t) {
    return tree;
  } else if (tree.token == Token::function) {
    return dispatch(tree);
  } else {
    return {Token::error, "ERROR: Can't eval tree token!", {}};
  }
}

auto tree_to_string(SyntaxTree tree) -> std::string {
  // TODO(yrom1): proper lists vs improper lists
  std::string output;
  print::prn(tree.token, tree.data);
  if (tree.token == Token::terminal || tree.token == Token::nil ||
      tree.token == Token::t) {
    return output += tree.data;
  } else {
    print::prn(tree.token, tree.data, tree.children.size());
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

auto _print(SyntaxTree tree) -> void { std::cout << tree_to_string(tree) << std::endl; }

auto print_tree(SyntaxTree tree) -> void {
  // TODO(yrom1) remove where-ever this is called with just print
  _print(tree);
}
auto eval_string_to_string(std::string input) -> std::string {
  return tree_to_string(eval(string_to_tree(input)));
}

// TODO(yrom1): list, quote, atom, eq, car, cdr, cons, cond
// see https://jtra.cz/stuff/lisp/sclr/index.html

auto run_tests() -> void {
  // assert(eval_string_to_string("") == "");
  assert(eval_string_to_string("1") == "1");
  assert(eval_string_to_string("42") == "42");

  assert(eval_string_to_string("()") == "()");  // no NIL sugar

  assert(eval_string_to_string("(list)") == "()");
  assert(eval_string_to_string("(list 1)") == "(list 1 ())");  // no (1) sugar
  assert(eval_string_to_string("(LIST 1)") ==
         "(list 1 ())");  // everything lowercase
  assert(eval_string_to_string("(list 1 2)") == "(list 1 2 ())");
  assert(eval_string_to_string("(list (list 1 2))") ==
         "(list (list 1 2 ()) ())");
  assert(eval_string_to_string("(list 1 (+ 2 3))") == "(list 1 5 ())");

  assert(eval_string_to_string("(cons 1 ())") == "(cons 1 ())");
  assert(eval_string_to_string("(cons 1 2)") == "(cons 1 2)");
  assert(eval_string_to_string("(cons () ())") == "(cons () ())");

  assert(eval_string_to_string("(atom ())") == "t");
  assert(eval_string_to_string("(atom (list 1))") == "()");
  assert(eval_string_to_string("(atom (cons 1 2))") == "()");

  // "(listp ())" == "t"

  assert(eval_string_to_string("(quote 1)") == "1");
  assert(eval_string_to_string("(quote (list 1 2))") == "(list 1 2)");
  assert(eval_string_to_string("(quote (+ 1 2))") == "(+ 1 2)");

  assert(eval_string_to_string("(car (list 1 2))") == "1");
  assert(eval_string_to_string("(car (list (list 1 2) 3 4))") ==
         "(list 1 2 ())");

  assert(eval_string_to_string("(cdr ())") == "()");
  assert(eval_string_to_string("(cdr (list 1))") == "()");
  assert(eval_string_to_string("(cdr (list 1 2))") == "(list 2 ())");
  assert(eval_string_to_string("(cdr (list (list 1 2) 3 4))") ==
         "(list 3 4 ())");

  assert(eval_string_to_string("(eq 1 1)") == "t");
  assert(eval_string_to_string("(eq 1 2)") == "()");
  assert(eval_string_to_string("(eq (list 1 2) (list 1 2))") == "t");
  assert(eval_string_to_string("(eq (list 1 (+ 2 3)) (list 1 (+ 2 3)))") ==
         "t");
  assert(eval_string_to_string("(eq (list 1 (+ 2 3)) (list 1 (+ 2 4)))") ==
         "()");

  // (cond (t)) -> (cond (list (list t)))
  // assert(eval_string_to_string("(cond)") == "()");
  // (cond (list)) is not allowed
  // assert(eval_string_to_string("(cond (list (list))") ==
  //  "()");  // no implicit list sugar, always pass (list (list... even when
  // unnecessary
  // assert(eval_string_to_string("(cond (list (list t))") == "t");
  // assert(eval_string_to_string("(cond (list (list 1))") == "1");
  // assert(eval_string_to_string("(cond (list (list ())))") == "()");
  // assert(eval_string_to_string("(cond (list (list t 1)))") == "1");
  // TODO(yrom1): put below tests in desugar notation
  // assert(eval_string_to_string("(cond (t 1))") == "1");
  // assert(eval_string_to_string("(cond (t 1 2 3 42))") == "42");
  // assert(eval_string_to_string("(cond (1 1))") == "1");
  // assert(eval_string_to_string("(cond (() 42) (t 1))") == "1");
  // assert(eval_string_to_string("(cond (t 1) (() 42))") == "1");
  // assert(eval_string_to_string("(cond (() 1))") == "()");
  // assert(eval_string_to_string("(cond (() 1) (() 2))") == "()");
  // assert(eval_string_to_string("(cond ((eq 1 2) 3) (t 4))") == "4");
  // assert(eval_string_to_string("(cond ((eq 1 2) 3) (() 4) (t 5))") ==
  // "5");

  assert(eval_string_to_string("(null 1)") == "()");
  assert(eval_string_to_string("(null ())") == "t");
  assert(eval_string_to_string("(null (cond))") == "t");

  // TODO(yrom1): code repetition with null and not tests
  assert(eval_string_to_string("(not 1)") == "()");
  assert(eval_string_to_string("(not ())") == "t");
  assert(eval_string_to_string("(not (cond))") == "t");

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
  // "100000000000000000000"); // dumps on stoi
  // assert(eval_string_to_string("(+ 1 ())") == "???"); // this core
  // dumps, pass error?
}

auto repl() -> void {
  while (true) {
    _print(eval(read()));
  }
}

int main() {
  run_tests();
  repl();
}
