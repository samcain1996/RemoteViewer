#pragma once

#include <algorithm>
#include <functional>

template <typename T, typename E = T::value_type>
using Condition = std::function<bool(T&, E&)>;

// Functions that represent common conditions
namespace DefaultConditions {

	template <typename T, typename E = T::value_type>
	bool In(const T& container, const E& element) {
		return std::find(container.begin(), container.end(), element) != container.end();
	}

	template <typename T, typename E = T::value_type>
	bool HasDup(const T& container, const E& element) {
		bool foundOnce = false;

		for (const E& elem : container) {
			if (elem == element) {
				if (foundOnce) { return true; }
				else { foundOnce = true; }
			}
		}

		return false;
	}

}
