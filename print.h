#ifndef PRINT_H_
#define PRINT_H_

#include <iostream>
#include <ranges>
#include <string>

namespace print {

namespace __detail {

template<typename T>
concept Printable = requires(T t) {
  { std::cout << t } -> std::same_as<std::ostream&>;
};

// start printItem

void printItem(const std::string& str) { std::cout << "\"" << str << "\""; }

void printItem(const char& c) { std::cout << "'" << c << "'"; }

template <Printable T>
void printItem(const T& item) {
  std::cout << item;
}

// end printItem

// start printHead

void printHead(const char* item) {
  std::string str{item};
  printItem(str);
}

void printHead(const std::string& item) {
  printItem(item);
}

template <Printable T>
void printHead(const T& item) {
  printItem(item);
}

template <Printable T, Printable U>
void printHead(const std::pair<T, U>& item) {
  std::cout << "(";
  printItem(item.first);
  std::cout << ", ";
  printItem(item.second);
  std::cout << ")";
}

template <std::ranges::range T>
void printHead(const T& container) {
  std::cout << '[';
  typename T::size_type idx = 0;
  for (const auto& elem : container) {
    ++idx;
    printHead(elem);
    // printItem(elem);
    if (idx != container.size()) {
      std::cout << ", ";
    }
  }
  std::cout << ']';
}

// end printHead

} // namespace __detail

// start pr

void pr() {}

template <typename HEAD, typename... TAIL>
void pr(const HEAD& head, TAIL... tail) {
  __detail::printHead(head);
  std::cout << ' ';
  pr(tail...);
}

// end pr

// start prn

void prn() { std::cout << '\n'; }

template <typename HEAD, typename... TAIL>
void prn(const HEAD& head, TAIL... tail) {
  __detail::printHead(head);
  std::cout << ' ';
  prn(tail...);
}

// end prn


} // namespace pr

#endif  // PRINT_H_
