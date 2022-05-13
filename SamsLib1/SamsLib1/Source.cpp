#include <iostream>
#include "Benchmarker.h"
#include <array>
#include <vector>

void someFunc() {
	char* arr = new char[1000000];
	std::cout << bench([&]() {
		srand(time(0));
		for (int i = 0; i < 1000000; i++) {
			arr[i] = rand();
		}
		}, std::micro());
	delete[] arr;
}

int main() {
	std::vector<int> nums;
	nums.reserve(1000000);
	std::cout << bench([&]() {
		srand(time(0));
		for (int i = 0; i < 1000000; i++) {
			nums[i] = rand();
		}
		}, std::micro()) << "\n";

	someFunc();

	return 0;
}