#include <iostream>
#include<type_traits>
#include<iosfwd>
#include<utility>

using namespace std;

template <typename T, typename U>
constexpr bool Convertible() { return std::is_convertible<T, U>::value; }
