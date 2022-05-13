#include <chrono>
#include <tuple>
#include <Windows.h>
#include <functional>

template <typename Func, typename Ratio>
auto bench(Func&& funcToBench, Ratio ratio)
requires (!std::is_same_v<typename decltype(std::function{funcToBench})::result_type, void >) {
	auto begin = std::chrono::high_resolution_clock::now();

	auto res = funcToBench();

	auto end = std::chrono::high_resolution_clock::now();

	return std::make_tuple(res, std::chrono::duration_cast<std::chrono::duration<long long, Ratio>>(end - begin).count());
}

template <typename Func, typename Ratio>
long long bench(Func&& funcToBench, Ratio ratio) 
requires (std::is_same_v<typename decltype(std::function{funcToBench})::result_type, void>) {

	auto begin = std::chrono::high_resolution_clock::now();
	funcToBench();
	auto end = std::chrono::high_resolution_clock::now();

	return std::chrono::duration_cast<std::chrono::duration<long long, Ratio>>(end - begin).count();
}

template <typename Func, typename Ratio>
auto bench(Func&& funcToBench, Ratio ratio, size_t trials)
{
	long long avgTime = 0;
	for (size_t i = 0; i < trials; i++)
	{
		auto [dummy, time] = bench(funcToBench, ratio);
		avgTime += time;
	}
	avgTime /= trials;
	return time;
}

template <typename Func, typename Ratio>
long long benchPrgm(Func&& funcToBench, Ratio ratio, const char* pgrmPath)
	requires (std::is_same_v<typename decltype(std::function{ funcToBench })::result_type, void > ) {

	auto begin = std::chrono::high_resolution_clock::now();
	funcToBench();
	auto end = std::chrono::high_resolution_clock::now();

	return std::chrono::duration_cast<std::chrono::duration<long long, Ratio>>(end - begin).count();
}
