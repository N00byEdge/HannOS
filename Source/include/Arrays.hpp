#pragma once

#include <array>

template<typename T, std::size_t sz, typename ...Args>
constexpr std::array<T, sz + 1> append(std::array<T, sz> arr, Args ...args) {
	std::array<T, sz + 1> ret(std::begin(arr), std::end(arr));
	std::get<sz>(ret) = std::forward<Args...>(args...);
}
