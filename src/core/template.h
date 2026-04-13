#pragma once

#include <cassert>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>

namespace gd {

using ld = long double;
using ll = long long;

#define all(x) x.begin(), x.end()
#define rall(x) x.rbegin(), x.rend()

class Comparator {
public:
	Comparator() : EPS(1e-9) {}
	Comparator(ld eps) : EPS(eps) {}

	int sgn(ld a) const {
		if (a > EPS) {
			return 1;
		} else if (a < -EPS) {
			return -1;
		} else {
			return 0;
		}
	}

	bool eq(ld a, ld b) const {
		return std::abs(a - b) <= EPS;
	}

	bool sml(ld a, ld b) const {
		return a + EPS < b;
	}

	bool grt(ld a, ld b) const {
		return a - EPS > b;
	}

	bool smleq(ld a, ld b) const {
		return a - EPS <= b;
	}

	bool grteq(ld a, ld b) const {
		return a + EPS >= b;
	}

	std::vector<ld> min(const std::vector<ld>& a, const std::vector<ld>& b) {
		assert(a.size() == b.size());
		std::vector<ld> res(a.size());
		for (int32_t i = 0; i < static_cast<int32_t>(a.size()); i++) {
			res[i] = std::min(a[i], b[i]);
		}
		return res;
	}

	std::vector<ld> max(const std::vector<ld>& a, const std::vector<ld>& b) {
		assert(a.size() == b.size());
		std::vector<ld> res(a.size());
		for (int32_t i = 0; i < static_cast<int32_t>(a.size()); i++) {
			res[i] = std::max(a[i], b[i]);
		}
		return res;
	}
private:
	ld EPS;
};

using Pt = std::vector<ld>;

inline std::ostream& operator<<(std::ostream& out, const Pt& x) {
	for (const auto& coord : x) {
		out << coord << ' ';
	}
	return out;
}

inline std::vector<ld> operator-(const std::vector<ld>& a, const std::vector<ld>& b) {
	assert(a.size() == b.size());
	std::vector<ld> res(a.size());
	for (int32_t i = 0; i < static_cast<int32_t>(a.size()); i++) {
		res[i] = a[i] - b[i];
	}
	return res;
}

inline std::vector<ld> operator+(const std::vector<ld>& a, const std::vector<ld>& b) {
	assert(a.size() == b.size());
	std::vector<ld> res(a.size());
	for (int32_t i = 0; i < static_cast<int32_t>(a.size()); i++) {
		res[i] = a[i] + b[i];
	}
	return res;
}

inline std::vector<ld> operator/(const std::vector<ld>& a, ld b) {
	assert(b != 0);
	std::vector<ld> res(a.size());
	for (int32_t i = 0; i < static_cast<int32_t>(a.size()); i++) {
		res[i] = a[i] / b;
	}
	return res;
}

inline std::vector<ld> operator*(const std::vector<ld>& a, ld b) {
	std::vector<ld> res(a.size());
	for (int32_t i = 0; i < static_cast<int32_t>(a.size()); i++) {
		res[i] = a[i] * b;
	}
	return res;
}

inline std::vector<ld>& operator+=(std::vector<ld>& a, const std::vector<ld>& b) {
	assert(a.size() == b.size());
	for (int32_t i = 0; i < static_cast<int32_t>(a.size()); i++) {
		a[i] += b[i];
	}
	return a;
}

inline std::vector<ld>& operator-=(std::vector<ld>& a, const std::vector<ld>& b) {
	assert(a.size() == b.size());
	for (int32_t i = 0; i < static_cast<int32_t>(a.size()); i++) {
		a[i] -= b[i];
	}
	return a;
}

} // namespace gd
