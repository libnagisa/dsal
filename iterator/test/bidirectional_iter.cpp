// Copyright (C) 2019 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <nagisa/dsal/iterator/iterator.h>

#include <algorithm>
#include <array>
#include <functional>
#include <numeric>
#include <list>
#include <type_traits>

struct basic_bidirectional_iter_impl
	: ::nagisa::dsal::iterator_interface<basic_bidirectional_iter_impl, ::std::bidirectional_iterator_tag, ::std::ptrdiff_t>
{
	using self_type = basic_bidirectional_iter_impl;

	basic_bidirectional_iter_impl() : it_(nullptr) {}
	basic_bidirectional_iter_impl(int* it) : it_(it) {}

	int& operator*() const { return *it_; }
	self_type& operator++()
	{
		++it_;
		return *this;
	}
	self_type& operator--()
	{
		--it_;
		return *this;
	}
	friend bool operator==(self_type lhs, self_type rhs) noexcept
	{
		return lhs.it_ == rhs.it_;
	}
private:
	int* it_;
};

using basic_bidirectional_iter = ::nagisa::dsal::iterator_adaptor<basic_bidirectional_iter_impl>;
static_assert(::std::same_as<::std::bidirectional_iterator_tag, ::nagisa::dsal::details::iter_concept_t<basic_bidirectional_iter_impl>>);
static_assert(::std::same_as<int, ::nagisa::dsal::details::iter_value_t<basic_bidirectional_iter_impl>>);
static_assert(::std::bidirectional_iterator<basic_bidirectional_iter>);
static_assert(::std::same_as<int, ::std::iter_value_t<basic_bidirectional_iter>>);

template<typename ValueType>
struct bidirectional_iter_impl
	: ::nagisa::dsal::iterator_interface<bidirectional_iter_impl<ValueType>,std::bidirectional_iterator_tag, ::std::ptrdiff_t, ValueType>
{
	using self_type = bidirectional_iter_impl;
	bidirectional_iter_impl() : it_(nullptr) {}
	bidirectional_iter_impl(ValueType* it) : it_(it) {}
	template<
		typename ValueType2,
		typename E = std::enable_if_t<
		std::is_convertible<ValueType2*, ValueType*>::value>>
		bidirectional_iter_impl(bidirectional_iter_impl<ValueType2> it) : it_(it.it_)
	{
	}

	ValueType& operator*() const { return *it_; }
	self_type& operator++()
	{
		++it_;
		return *this;
	}
	self_type& operator--()
	{
		--it_;
		return *this;
	}
	friend bool
		operator==(self_type lhs, self_type rhs) noexcept
	{
		return lhs.it_ == rhs.it_;
	}
private:
	ValueType* it_;

	template<typename ValueType2>
	friend struct bidirectional_iter_impl;
};
template<typename ValueType>
using bidirectional_iter = ::nagisa::dsal::iterator_adaptor<bidirectional_iter_impl<ValueType>>;
using bidirectional = bidirectional_iter<int>;
using const_bidirectional = bidirectional_iter<int const>;
static_assert(::std::bidirectional_iterator<bidirectional>);
static_assert(::std::bidirectional_iterator<const_bidirectional>);

#include <cassert>
#define BOOST_TEST assert

int main()
{
	std::array<int, 10> ints = { {0, 1, 2, 3, 4, 5, 6, 7, 8, 9} };
	{
		basic_bidirectional_iter first(ints.data());
		basic_bidirectional_iter last(ints.data() + ints.size());
		{
			std::array<int, 10> ints_copy;
			std::copy(first, last, ints_copy.begin());
			BOOST_TEST(ints_copy == ints);
		}

		{
			std::array<int, 10> ints_copy;
			std::copy(
				std::make_reverse_iterator(last),
				std::make_reverse_iterator(first),
				ints_copy.begin());
			std::reverse(ints_copy.begin(), ints_copy.end());
			BOOST_TEST(ints_copy == ints);
		}

		{
			std::array<int, 10> iota_ints;
			basic_bidirectional_iter first(iota_ints.data());
			basic_bidirectional_iter last(iota_ints.data() + iota_ints.size());
			std::iota(first, last, 0);
			BOOST_TEST(iota_ints == ints);
		}

		{
			std::array<int, 10> iota_ints;
			basic_bidirectional_iter first(iota_ints.data());
			basic_bidirectional_iter last(iota_ints.data() + iota_ints.size());
			std::iota(
				std::make_reverse_iterator(last),
				std::make_reverse_iterator(first),
				0);
			std::reverse(iota_ints.begin(), iota_ints.end());
			BOOST_TEST(iota_ints == ints);
		}
	}


	{
		{
			bidirectional first(ints.data());
			bidirectional last(ints.data() + ints.size());
			while (first != last && !(first == last))
				first++;
		}

		{
			bidirectional first(ints.data());
			bidirectional last(ints.data() + ints.size());
			while (first != last && !(first == last))
				last--;
		}

		{
			basic_bidirectional_iter first(ints.data());
			basic_bidirectional_iter last(ints.data() + ints.size());
			while (first != last && !(first == last))
				first++;
		}

		{
			basic_bidirectional_iter first(ints.data());
			basic_bidirectional_iter last(ints.data() + ints.size());
			while (first != last && !(first == last))
				last--;
		}
	}


	{
		bidirectional first(ints.data());
		bidirectional last(ints.data() + ints.size());

		{
			std::array<int, 10> ints_copy;
			std::copy(first, last, ints_copy.begin());
			BOOST_TEST(ints_copy == ints);
		}

		{
			std::array<int, 10> ints_copy;
			std::copy(
				std::make_reverse_iterator(last),
				std::make_reverse_iterator(first),
				ints_copy.begin());
			std::reverse(ints_copy.begin(), ints_copy.end());
			BOOST_TEST(ints_copy == ints);
		}

		{
			std::array<int, 10> iota_ints;
			bidirectional first(iota_ints.data());
			bidirectional last(iota_ints.data() + iota_ints.size());
			std::iota(first, last, 0);
			BOOST_TEST(iota_ints == ints);
		}

		{
			std::array<int, 10> iota_ints;
			bidirectional first(iota_ints.data());
			bidirectional last(iota_ints.data() + iota_ints.size());
			std::iota(
				std::make_reverse_iterator(last),
				std::make_reverse_iterator(first),
				0);
			std::reverse(iota_ints.begin(), iota_ints.end());
			BOOST_TEST(iota_ints == ints);
		}
	}


	{
		const_bidirectional first(ints.data());
		const_bidirectional last(ints.data() + ints.size());

		{
			std::array<int, 10> ints_copy;
			std::copy(first, last, ints_copy.begin());
			BOOST_TEST(ints_copy == ints);
		}

		{
			BOOST_TEST(std::binary_search(first, last, 3));
			BOOST_TEST(std::binary_search(
				std::make_reverse_iterator(last),
				std::make_reverse_iterator(first),
				3,
				std::greater<>{}));
		}
	}

	return 0;
}