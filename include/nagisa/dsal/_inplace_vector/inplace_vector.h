#pragma once

#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

#include "./push_expect.h"
#include "./push_throw.h"

inline constexpr struct from_range_t {} from_range{};

template <::std::size_t N>
using smallest_size_t =
	::std::conditional_t<(N < ::std::numeric_limits<::std::uint8_t>::max()), ::std::uint8_t,
	::std::conditional_t<(N < ::std::numeric_limits<::std::uint16_t>::max()), ::std::uint16_t,
	::std::conditional_t<(N < ::std::numeric_limits<::std::uint32_t>::max()), ::std::uint32_t,
	::std::conditional_t<(N < ::std::numeric_limits<::std::uint64_t>::max()), ::std::uint64_t,
	::std::size_t>>>>;

// Index a random-access and sized range doing bound checks in debug builds
template <::std::ranges::random_access_range Rng>
constexpr decltype(auto) index(Rng&& rng, ::std::integral auto i) noexcept
	requires(::std::ranges::sized_range<Rng>)
{
	NAGISA_DSAL_EXPECT(static_cast<::std::ptrdiff_t>(i) < ::std::ranges::size(rng), "out of range");
	return ::std::ranges::begin(NAGISA_STL_FREESTANDING_UTILITY_FORWARD(rng))[NAGISA_STL_FREESTANDING_UTILITY_FORWARD(i)];
}

// http://eel.is/c++draft/container.requirements.general#container.intro.reqmts-2
template <class Rng, class T>
concept container_compatible_range =
	::std::ranges::input_range<Rng>
	&& ::std::convertible_to<::std::ranges::range_reference_t<Rng>, T>;

template <class T, ::std::size_t N>
concept satify_constexpr = N == 0 || ::std::is_trivial_v<T>;

template <class T>
concept lessthan_comparable = requires(T const& a, T const& b)
{
	{ a < b } -> ::std::convertible_to<bool>;
};

template <class T>
struct zero_sized
{
	using size_type = ::std::uint8_t;
	static constexpr auto _storage_data() noexcept { return static_cast<T*>(nullptr); }
	static constexpr auto _storage_size() noexcept { return static_cast<size_type>(0); }
	static constexpr auto _unsafe_set_size([[maybe_unused]] ::std::size_t new_size) noexcept
	{
		NAGISA_DSAL_EXPECT(new_size == 0, "out of range");
	}
};

template <class T, ::std::size_t N>
struct trivial
{
	static_assert(::std::is_trivially_default_constructible_v<T> && ::std::is_trivially_copyable_v<T>, "storage::trivial<T, C> requires Trivial<T>");
	static_assert(N != ::std::size_t{ 0 }, "N == 0, use zero_sized");

	using size_type = smallest_size_t<N>;

	// If value_type is const, then const ::std::array of non-const elements:
	alignas(T) type_traits::add_const_like_t<::std::array<::std::remove_const_t<T>, N>, T> _data {};
	size_type _size = 0;

	constexpr auto _storage_data() const noexcept { return _data.data(); }
	constexpr auto _storage_data() noexcept { return _data.data(); }
	constexpr auto _storage_size() const noexcept { return _size; }
	constexpr auto _unsafe_set_size(::std::size_t new_size) noexcept
	{
		NAGISA_DSAL_EXPECT(size_type(new_size) <= N, "out of range");
		_size = size_type(new_size);
	}

	constexpr trivial() noexcept = default;
};

/// Storage for non-trivial elements.
template <class T, ::std::size_t N>
struct non_trivial
{
	static_assert(!::std::is_trivial_v<T>, "use storage::trivial for Trivial<T> elements");
	static_assert(N != size_t{ 0 }, "use storage::zero for N==0");

	using size_type = smallest_size_t<N>;

	alignas(T) type_traits::add_const_like_t<::std::array<::std::byte, sizeof(T) * N>, T> _data{}; // BUGBUG: test SIMD types
	size_type _size = 0;

	constexpr auto _storage_data() const noexcept { return reinterpret_cast<T const*>(_data.data()); }
	constexpr auto _storage_data() noexcept { return reinterpret_cast<T*>(_data.data()); }
	constexpr auto _storage_size() const noexcept { return _size; }
	constexpr void _unsafe_set_size(size_t new_size) noexcept
	{
		NAGISA_DSAL_EXPECT(size_type(new_size) <= N, "out of range");
		_size = size_type(new_size);
	}

	constexpr non_trivial() noexcept = default;
	constexpr ~non_trivial() requires(::std::is_trivially_destructible_v<T>) = default;
	constexpr ~non_trivial()
	{
		::std::destroy(_storage_data(), _storage_data() + _storage_size());
	}
};

// Selects the vector storage.
template <class T, ::std::size_t N>
using storage_for = ::std::conditional_t<
	!satify_constexpr<T, N>, non_trivial<T, N>,
	::std::conditional_t<N == 0, zero_sized<T>, trivial<T, N>>>;


template <class T, ::std::size_t N>
struct inplace_vector : storage_for<T, N>
{
private:
	static_assert(::std::is_nothrow_destructible_v<T>, "T must be nothrow destructible");
	using base_type = storage_for<T, N>;
	using self_type = inplace_vector;
public:
	using value_type = T;
	using pointer = T*;
	using const_pointer = T const*;
	using reference = value_type&;
	using const_reference = value_type const&;
	using size_type = ::std::size_t;
	using difference_type = ::std::ptrdiff_t;
	using iterator = pointer;
	using const_iterator = const_pointer;
	using reverse_iterator = ::std::reverse_iterator<iterator>;
	using const_reverse_iterator = ::std::reverse_iterator<const_iterator>;

	constexpr iterator begin() noexcept { return base_type::_storage_data(); }
	constexpr const_iterator begin() const noexcept { return base_type::_storage_data(); }
	constexpr iterator end() noexcept { return begin() + size(); }
	constexpr const_iterator end() const noexcept { return begin() + size(); }
	constexpr reverse_iterator rbegin() noexcept {
		return reverse_iterator(end());
	}
	constexpr const_reverse_iterator rbegin() const noexcept {
		return const_reverse_iterator(end());
	}
	constexpr reverse_iterator rend() noexcept {
		return reverse_iterator(begin());
	}
	constexpr const_reverse_iterator rend() const noexcept {
		return const_reverse_iterator(begin());
	}

	constexpr const_iterator cbegin() const noexcept { return base_type::_storage_data(); }
	constexpr const_iterator cend() const noexcept { return cbegin() + size(); }
	constexpr const_reverse_iterator crbegin() const noexcept {
		return const_reverse_iterator(cend());
	}
	constexpr const_reverse_iterator crend() const noexcept {
		return const_reverse_iterator(cbegin());
	}

	// [inplace.vector.capacity], size/capacity
	[[nodiscard]] constexpr bool empty() const noexcept { return base_type::_storage_size() == 0; }
	[[nodiscard]] constexpr size_type size() const noexcept { return base_type::_storage_size(); }
	constexpr void shrink_to_fit() {}
	static constexpr size_type max_size() noexcept { return N; }
	static constexpr size_type capacity() noexcept { return N; }

	constexpr reference operator[](size_type n) { return details::index(*this, n); }
	constexpr const_reference operator[](size_type n) const { return details::index(*this, n); }
	constexpr reference front() { return details::index(*this, static_cast<size_type>(0)); }
	constexpr const_reference front() const { return details::index(*this, static_cast<size_type>(0)); }
	constexpr reference back() { return details::index(*this, size() - static_cast<size_type>(1)); }
	constexpr const_reference back() const { return details::index(*this, size() - static_cast<size_type>(1)); }

	// [containers.sequences.inplace_vector.data], data access
	constexpr pointer data() noexcept { return base_type::_storage_data(); }
	constexpr const_pointer data() const noexcept { return base_type::_storage_data(); }

	constexpr void _assert_iterator_in_range([[maybe_unused]] const_iterator it) noexcept
	{
		NAGISA_DSAL_EXPECT(begin() <= it, "iterator not in range");
		NAGISA_DSAL_EXPECT(it <= end(), "iterator not in range");
	}
	constexpr void _assert_valid_iterator_pair([[maybe_unused]] const_iterator first, [[maybe_unused]] const_iterator last) noexcept
	{
		NAGISA_DSAL_EXPECT(first <= last, "invalid iterator pair");
	}
	constexpr void _assert_iterator_pair_in_range(const_iterator first, const_iterator last) noexcept
	{
		self_type::_assert_iterator_in_range(first);
		self_type::_assert_iterator_in_range(last);
		self_type::_assert_valid_iterator_pair(first, last);
	}
	constexpr void _unsafe_destroy(iterator first, iterator last) noexcept(::std::is_nothrow_destructible_v<T>)
	{
		self_type::_assert_iterator_pair_in_range(first, last);
		if constexpr (N > 0 && !std::is_trivial_v<T>) 
		{
			for (; first != last; ++first)
				first->~T();
		}
	}

	// [inplace.vector.modifiers], modifiers
	template <class... Args>
	constexpr reference unchecked_emplace_back(Args&&...args)
		requires ::std::constructible_from<T, Args...>
	{
		NAGISA_DSAL_EXPECT(size() < capacity(), "inplace_vector out-of-memory");
		::std::construct_at(end(), NAGISA_STL_FREESTANDING_UTILITY_FORWARD(args)...);
		base_type::_unsafe_set_size(size() + static_cast<size_type>(1));
		return this->back();
	}

	template <class... Args>
	constexpr pointer try_emplace_back(Args&&... args)
	{
		if (size() == capacity()) [[unlikely]]
			return nullptr;
		return ::std::addressof(self_type::unchecked_emplace_back(NAGISA_STL_FREESTANDING_UTILITY_FORWARD(args)...));
	}

	constexpr pointer try_push_back(T const& x)
		requires ::std::constructible_from<T, T const&>
	{
		return self_type::try_emplace_back(x);
	}
	constexpr pointer try_push_back(T&& x)
		requires ::std::constructible_from<T, T&&>
	{
		return self_type::try_emplace_back(::std::move(x));
	}

	constexpr reference unchecked_push_back(T const& x)
		requires ::std::constructible_from<T, T const&>
	{
		return self_type::unchecked_emplace_back(x);
	}
	constexpr reference unchecked_push_back(T&& x)
		requires ::std::constructible_from<T, T&&>
	{
		return self_type::unchecked_emplace_back(::std::move(x));
	}

	template<container_compatible_range<T> R>
	constexpr ::std::ranges::borrowed_iterator_t<R> try_append_range(R&& rg)
		requires ::std::constructible_from<T, ::std::ranges::range_reference_t<R>>
	{
		auto it = ::std::ranges::begin(rg);
		auto end = ::std::ranges::end(rg);
		for (; size() != capacity() && it != end; ++it) 
		{
			self_type::unchecked_emplace_back(*it);
		}
		return it;
	}

	constexpr iterator erase(const_iterator first, const_iterator last)
		requires ::std::movable<T>
	{
		self_type::_assert_iterator_pair_in_range(first, last);
		auto f = begin() + (first - begin());
		if (first != last) {
			self_type::_unsafe_destroy(::std::ranges::move(f + (last - first), end(), f), end());
			base_type::_unsafe_set_size(size() - static_cast<size_type>(last - first));
		}
		return f;
	}

	constexpr iterator erase(const_iterator position)
		requires ::std::movable<T>
	{
		return self_type::erase(position, position + 1);
	}

	constexpr void clear() noexcept
	{
		self_type::_unsafe_destroy(begin(), end());
		base_type::_unsafe_set_size(0);
	}

	constexpr void pop_back()
	{
		NAGISA_DSAL_EXPECT(size() > 0, "pop_back from empty inplace_vector!");
		self_type::_unsafe_destroy(end() - 1, end());
		base_type::_unsafe_set_size(size() - 1);
	}

	friend constexpr bool operator==(self_type const& x, self_type const& y)
	{
		return x.size() == y.size() && ::std::ranges::equal(x, y);
	}

	constexpr void swap(self_type& x) noexcept(N == 0 || (::std::is_nothrow_swappable_v<T> && ::std::is_nothrow_move_constructible_v<T>))
		requires ::std::movable<T>
	{
		auto tmp = ::std::move(x);
		x = ::std::move(*this);
		(*this) = ::std::move(tmp);
	}

	friend constexpr void swap(self_type& x, self_type& y) noexcept(N == 0 || (::std::is_nothrow_swappable_v<T> && ::std::is_nothrow_move_constructible_v<T>))
	{
		x.swap(y);
	}

	friend constexpr auto operator<=>(self_type const& x, self_type const& y)
		requires lessthan_comparable<T>
	{
		if constexpr (::std::three_way_comparable<T>) {
			return ::std::lexicographical_compare_three_way(x.begin(), x.end(),
				y.begin(), y.end());
		}
		else {
			auto sz = ::std::min(x.size(), y.size());
			for (auto i = static_cast<::std::size_t>(0); i < sz; ++i) {
				if (x[i] < y[i])
					return ::std::strong_ordering::less;
				if (y[i] < x[i])
					return ::std::strong_ordering::greater;
				// [container.opt.reqmts] < must be total ordering relationship
			}

			return x.size() <=> y.size();
		}
	}

	// [containers.sequences.inplace_vector.cons], construct/copy/destroy

	constexpr inplace_vector() noexcept = default;

	constexpr inplace_vector(self_type const& x) requires(N == 0 || ::std::is_trivially_copy_constructible_v<T>) = default;

	constexpr inplace_vector(self_type const& x)
		requires(N != 0) && (!::std::is_trivially_copy_constructible_v<T>) && ::std::copyable<T>
	{
		for (auto&& e : x)
			self_type::unchecked_emplace_back(e);
	}

	constexpr inplace_vector(self_type&& x) requires(N == 0 || ::std::is_trivially_move_constructible_v<T>) = default;

	constexpr inplace_vector(self_type&& x)
		requires (N != 0) && (!::std::is_trivially_move_constructible_v<T>) && ::std::movable<T>
	{
		for (auto&& e : x)
			self_type::unchecked_emplace_back(::std::move(e));
	}

	constexpr self_type& operator=(self_type const& x)
		requires (N == 0)
		|| (::std::is_trivially_destructible_v<T> && ::std::is_trivially_copy_constructible_v<T> && ::std::is_trivially_copy_assignable_v<T>)
		= default;

	constexpr self_type& operator=(self_type const& x)
		requires (N != 0)
		&& (!std::is_trivially_destructible_v<T>)
		&& (::std::is_trivially_copy_constructible_v<T>)
		&& (::std::is_trivially_copy_assignable_v<T>)
		&& ::std::copyable<T>
	{
		clear();
		for (auto&& e : x)
			self_type::unchecked_emplace_back(e);
		return *this;
	}

	constexpr self_type& operator=(self_type&& x)
		requires (N == 0)
		|| (::std::is_trivially_destructible_v<T> && ::std::is_trivially_move_constructible_v<T> && ::std::is_trivially_move_assignable_v<T>)
		= default;

	constexpr self_type& operator=(self_type&& x)
		requires(N != 0)
		&& (!::std::is_trivially_destructible_v<T>)
		&& (::std::is_trivially_move_constructible_v<T>)
		&& (::std::is_trivially_move_assignable_v<T>)
		&& ::std::movable<T>
	{
		clear();
		for (auto&& e : x)
			self_type::unchecked_emplace_back(::std::move(e));
		return *this;
	}

	//

	constexpr reference emplace_back(auto&&... args)
		requires ::std::constructible_from<T, decltype(args)...>
	{
		if (!self_type::try_emplace_back(NAGISA_STL_FREESTANDING_UTILITY_FORWARD(args)...)) [[unlikely]]
		{
			NAGISA_DSAL_THROW(::std::bad_alloc());
		}
		return self_type::back();
	}
	constexpr reference push_back(T const& x)
		requires ::std::constructible_from<T, T const&>
	{
		self_type::emplace_back(x);
		return self_type::back();
	}
	constexpr reference push_back(T&& x)
		requires ::std::constructible_from<T, T&&>
	{
		self_type::emplace_back(::std::move(x));
		return self_type::back();
	}

	template<container_compatible_range<T> R>
	constexpr void append_range(R&& rg)
		requires ::std::constructible_from<T, ::std::ranges::range_reference_t<R>>
	{
		if constexpr (::std::ranges::sized_range<R>) {
			if (self_type::size() + ::std::ranges::size(rg) > self_type::capacity()) [[unlikely]]
			{
				NAGISA_DSAL_THROW(::std::bad_alloc());
			}
		}
		for (auto&& e : rg) 
		{
			if (self_type::size() == self_type::capacity()) [[unlikely]]
			{
				NAGISA_DSAL_THROW(::std::bad_alloc());
			}
			self_type::emplace_back(NAGISA_STL_FREESTANDING_UTILITY_FORWARD(e));
		}
	}

	constexpr iterator emplace(const_iterator position, auto&&... args)
		requires ::std::constructible_from<T, decltype(args)...>&& ::std::movable<T>
	{
		self_type::_assert_iterator_in_range(position);
		auto b = self_type::end();
		self_type::emplace_back(NAGISA_STL_FREESTANDING_UTILITY_FORWARD(args)...);
		auto pos = self_type::begin() + (position - self_type::begin());
		::std::ranges::rotate(pos, b, self_type::end());
		return pos;
	}

	template <class InputIterator>
	constexpr iterator insert(const_iterator position, InputIterator first, InputIterator last)
		requires ::std::constructible_from<T, ::std::iter_reference_t<InputIterator>> && ::std::movable<T>
	{
		self_type::_assert_iterator_in_range(position);
		if constexpr (::std::random_access_iterator<InputIterator>) 
		{
			if (self_type::size() + static_cast<size_type>(::std::ranges::distance(first, last)) > self_type::capacity()) [[unlikely]]
			{
				NAGISA_DSAL_THROW(::std::bad_alloc());
			}
		}
		auto b = self_type::end();
		for (; first != last; ++first)
			self_type::emplace_back(::std::move(*first));
		auto pos = self_type::begin() + (position - self_type::begin());
		::std::ranges::rotate(pos, b, self_type::end());
		return pos;
	}

	template<container_compatible_range<T> R>
	constexpr iterator insert_range(const_iterator position, R&& rg)
		requires ::std::constructible_from<T, ::std::ranges::range_reference_t<R>> && ::std::movable<T>
	{
		return self_type::insert(position, ::std::ranges::begin(rg), ::std::ranges::end(rg));
	}

	constexpr iterator insert(const_iterator position, ::std::initializer_list<T> il)
		requires ::std::constructible_from<T, ::std::ranges::range_reference_t<::std::initializer_list<T>>> && ::std::movable<T>
	{
		return self_type::insert_range(position, il);
	}

	constexpr iterator insert(const_iterator position, size_type n, T const& x)
		requires ::std::constructible_from<T, T const&> && ::std::copyable<T>
	{
		self_type::_assert_iterator_in_range(position);
		auto b = self_type::end();
		for (size_type i = 0; i < n; ++i)
			self_type::emplace_back(x);
		auto pos = self_type::begin() + (position - self_type::begin());
		::std::ranges::rotate(pos, b, self_type::end());
		return pos;
	}

	constexpr iterator insert(const_iterator position, T const& x)
		requires ::std::constructible_from<T, T const&>&& ::std::copyable<T>
	{
		return self_type::insert(position, 1, x);
	}

	constexpr iterator insert(const_iterator position, T&& x)
		requires ::std::constructible_from<T, T&&> && ::std::movable<T>
	{
		return self_type::emplace(position, ::std::move(x));
	}

	constexpr inplace_vector& operator=(::std::initializer_list<T> il)
		requires ::std::constructible_from<T, ::std::ranges::range_reference_t<::std::initializer_list<T>>> && ::std::movable<T>
	{
		self_type::assign_range(il);
		return *this;
	}

	template <class InputIterator>
	constexpr void assign(InputIterator first, InputIterator last)
		requires ::std::constructible_from<T, ::std::iter_reference_t<InputIterator>> && ::std::movable<T>
	{
		self_type::clear();
		self_type::insert(self_type::begin(), first, last);
	}
	template <container_compatible_range<T> R>
	constexpr void assign_range(R&& rg)
		requires ::std::constructible_from<T, ::std::ranges::range_reference_t<R>> && ::std::movable<T>
	{
		self_type::assign(::std::begin(rg), ::std::end(rg));
	}
	constexpr void assign(size_type n, T const& u)
		requires ::std::constructible_from<T, T const&>&& ::std::movable<T>
	{
		self_type::clear();
		self_type::insert(self_type::begin(), n, u);
	}
	constexpr void assign(::std::initializer_list<T> il)
		requires ::std::constructible_from<T, ::std::ranges::range_reference_t<std::initializer_list<T>>> && ::std::movable<T>
	{
		self_type::clear();
		self_type::insert_range(self_type::begin(), il);
	}

	// [inplace.vector.capacity], size/capacity

	constexpr void reserve(size_type n)
	{
		if (n > N) [[unlikely]]
		{
			NAGISA_DSAL_THROW(::std::bad_alloc());
		}
	}

	constexpr void resize(size_type sz, T const& c)
		requires ::std::constructible_from<T, T const&>&& ::std::copyable<T>
	{
		if (sz == self_type::size())
			return;
		if (sz > N) [[unlikely]]
		{
			NAGISA_DSAL_THROW(::std::bad_alloc());
		}
		if (sz > self_type::size())
		{
			self_type::insert(self_type::end(), sz - self_type::size(), c);
			return;
		}
		self_type::_unsafe_destroy(self_type::begin() + sz, self_type::end());
		base_type::_unsafe_set_size(sz);
	}
	constexpr void resize(size_type sz)
		requires ::std::constructible_from<T, T&&>&& ::std::default_initializable<T>
	{
		if (sz == self_type::size())
			return;
		if (sz > N) [[unlikely]]
		{
			NAGISA_DSAL_THROW(::std::bad_alloc());
		}
		if (sz > self_type::size()) 
		{
			while (self_type::size() != sz)
				self_type::emplace_back(T{});
			return;
		}
		self_type::_unsafe_destroy(self_type::begin() + sz, self_type::end());
		base_type::_unsafe_set_size(sz);
	}

	// element access
	constexpr reference at(size_type pos)
	{
		if (pos >= self_type::size()) [[unlikely]]
		{
			NAGISA_DSAL_THROW(::std::out_of_range("inplace_vector::at"));
		}
		return details::index(*this, pos);
	}
	constexpr const_reference at(size_type pos) const
	{
		if (pos >= self_type::size()) [[unlikely]]
		{
			NAGISA_DSAL_THROW(::std::out_of_range("inplace_vector::at"));
		}
		return details::index(*this, pos);
	}

	// [containers.sequences.inplace_vector.cons], construct/copy/destroy

	constexpr inplace_vector(::std::initializer_list<T> il)
		requires ::std::constructible_from<T, ::std::ranges::range_reference_t<std::initializer_list<T>>> && ::std::movable<T>
	{
		self_type::insert(self_type::begin(), il);
	}

	constexpr inplace_vector(size_type n, T const & value)
		requires ::std::constructible_from<T, T const&>&& ::std::copyable<T>
	{
		self_type::insert(self_type::begin(), n, value);
	}

	constexpr explicit inplace_vector(size_type n)
		requires ::std::constructible_from<T, T&&> && ::std::default_initializable<T>
	{
		for (size_type i = 0; i < n; ++i)
			self_type::emplace_back(T{});
	}

	template <class InputIterator> // BUGBUG: why not ::std::ranges::input_iterator?
	constexpr inplace_vector(InputIterator first, InputIterator last)
		requires ::std::constructible_from<T, ::std::iter_reference_t<InputIterator>> && ::std::movable<T>
	{
		self_type::insert(self_type::begin(), first, last);
	}

	template <container_compatible_range<T> R>
	constexpr inplace_vector(from_range_t, R&& rg)
		requires ::std::constructible_from<T, ::std::ranges::range_reference_t<R>> && ::std::movable<T>
	{
		self_type::insert_range(self_type::begin(), NAGISA_STL_FREESTANDING_UTILITY_FORWARD(rg));
	}
};


template <class T, ::std::size_t N, class U = T>
constexpr ::std::size_t erase(inplace_vector<T, N>& c, U const& value)
{
	auto it = ::std::ranges::remove(c.begin(), c.end(), value);
	auto r = ::std::ranges::distance(it, c.end());
	c.erase(it, c.end());
	return r;
}

template <class T, ::std::size_t N, class Predicate>
constexpr ::std::size_t erase_if(inplace_vector<T, N>& c, Predicate pred)
{
	auto it = ::std::ranges::remove_if(c.begin(), c.end(), pred);
	auto r = ::std::ranges::distance(it, c.end());
	c.erase(it, c.end());
	return r;
}

#include "./pop_expect.h"
#include "./pop_throw.h"

NAGISA_BUILD_LIB_DETAIL_END
