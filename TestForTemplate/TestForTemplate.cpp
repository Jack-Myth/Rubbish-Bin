template<typename Sig>
struct get_;

template<typename R, typename... Args>
struct get_<R(*)(Args...)> {
	static size_t const value = sizeof...(Args);
};

template<typename Sig>
inline size_t get(Sig) {
	return get_<Sig>::value;
}

void fun(int, int) {}

#include <iostream>
int main() {
	std::cout << "fun的参数个数" << get(fun) << "\n";

	return 0;
}