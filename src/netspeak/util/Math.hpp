#ifndef NETSPEAK_UTIL_MATH_HPP
#define NETSPEAK_UTIL_MATH_HPP


namespace netspeak {
namespace util {

template <typename T>
T binomial(T n, T k) {
  // https://stackoverflow.com/a/15302448/7595472
  if (k == 0) {
    return 1;
  } else {
    return n * binomial(n - 1, k - 1) / k;
  }
}

template <typename T>
T factorial(T n) {
  T prod = 1;
  for (; n > 1; n--) {
    prod *= n;
  }
  return prod;
}

} // namespace util
} // namespace netspeak


#endif
