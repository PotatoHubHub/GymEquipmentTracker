#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

// this is where we keep inventory
template <typename T>
class Inventory {
	static_assert(!std::is_polymorphic<T>::value || std::has_virtual_destructor<T>::value,
		"Inventory<T>: polymorphic T needs a virtual destructor so deletion through unique_ptr<T> is safe.");

private:
	std::vector<std::unique_ptr<T>> items;

	void checkIndex(std::size_t index) const {
		if (index >= items.size()) {
			throw std::out_of_range("Inventory index out of range.");
		}
	}

public:
	using const_iterator = typename std::vector<std::unique_ptr<T>>::const_iterator;

	Inventory() = default;

	// ownership stuff
	Inventory(const Inventory&) = delete;
	Inventory& operator=(const Inventory&) = delete;
	Inventory(Inventory&&) noexcept = default;
	Inventory& operator=(Inventory&&) noexcept = default;   

	// --- modifying ---
	void add(std::unique_ptr<T> item) {
		if (!item) throw std::invalid_argument("Cannot add a null item.");
		items.push_back(std::move(item));
	}

	// erasing the unique_ptr destroys the T it owns
	void removeAt(std::size_t index) {
		checkIndex(index);
		items.erase(items.begin() + static_cast<std::ptrdiff_t>(index));
	}

	// --- access ---
	std::size_t size() const { return items.size(); }
	bool empty() const { return items.empty(); }

	T& at(std::size_t index) { checkIndex(index); return *items[index]; }
	const T& at(std::size_t index) const { checkIndex(index); return *items[index]; }

	const_iterator begin() const { return items.begin(); }
	const_iterator end() const { return items.end(); }

	// --- searching (Pred is any callable taking const T&, returning bool) ---
	template <typename Pred>
	std::optional<std::size_t> findIndex(Pred pred) const {
		for (std::size_t i = 0; i < items.size(); ++i) {
			if (pred(*items[i])) return i;
		}
		return std::nullopt;
	}

	template <typename Pred>
	bool contains(Pred pred) const { return findIndex(pred).has_value(); }

	template <typename Pred>
	std::size_t countIf(Pred pred) const {
		std::size_t n = 0;
		for (const auto& item : items) {
			if (pred(*item)) ++n;
		}
		return n;
	}

	template <typename Pred>
	std::vector<std::size_t> findAll(Pred pred) const {
		std::vector<std::size_t> results;
		for (std::size_t i = 0; i < items.size(); ++i) {
			if (pred(*items[i])) results.push_back(i);
		}
		return results;
	}
};
