#include <nagisa/dsal/iterator/iterator.h>
#include <string>
#include <memory>
#include <cstddef>

struct input_or_output_impl : ::nagisa::dsal::iterator_interface<input_or_output_impl, void, ::std::ptrdiff_t>
{
	using self_type = input_or_output_impl;
	::std::unique_ptr<int> value = ::std::make_unique<int>(0);
	mutable ::std::size_t dereference_count = 0;

	self_type& operator++()
	{
		++(*value);
		return *this;
	}
	auto operator*() const
	{
		++dereference_count;
		return dereference_count;
	}
};
using input_or_output_iterator = ::nagisa::dsal::iterator_adaptor<input_or_output_impl>;
static_assert(::std::input_or_output_iterator<input_or_output_iterator>);
static_assert(!::std::copy_constructible<input_or_output_iterator>);
static_assert(::std::same_as<void, decltype(::std::declval<input_or_output_iterator>()++)>);

struct output_impl : ::nagisa::dsal::iterator_interface<output_impl, ::std::output_iterator_tag, ::std::ptrdiff_t>
{
	using self_type = output_impl;

	::std::unique_ptr<int> value = ::std::make_unique<int>(0);
	mutable ::std::size_t dereference_count = 0;

	self_type& operator++()
	{
		++(*value);
		return *this;
	}
	struct observer
	{
		int* v;

		auto operator=(int v) const { *this->v = v; }
		auto operator*() const { return observer{ *this }; }
	};
	auto operator++(int) const
	{
		++(*value);
		return observer{ &*value };
	}
	auto operator*() const
	{
		return observer{ &*value };
	}
};
using output_iterator = ::nagisa::dsal::iterator_adaptor<output_impl>;
static_assert(::std::output_iterator<output_iterator, int>);
static_assert(::std::output_iterator<::nagisa::dsal::iterator_adaptor<::std::ostream_iterator<int>>, int>);
static_assert(::std::output_iterator<::nagisa::dsal::iterator_adaptor<::std::ostreambuf_iterator<int>>, int>);

struct input_impl : ::nagisa::dsal::iterator_interface<input_impl, ::std::input_iterator_tag, ::std::ptrdiff_t>
{
	using self_type = input_impl;
	::std::unique_ptr<int> value = ::std::make_unique<int>(0);
	mutable ::std::size_t dereference_count = 0;

	auto operator*() const
	{
		return 0;
	}
	self_type& operator++()
	{
		return *this;
	}
};
using input_iterator = ::nagisa::dsal::iterator_adaptor<input_impl>;
static_assert(::std::input_iterator<input_iterator>);
static_assert(::std::input_iterator<::nagisa::dsal::iterator_adaptor<::std::istream_iterator<char>>>);
static_assert(::std::input_iterator<::std::istreambuf_iterator<char>>);
static_assert(::std::input_iterator<::nagisa::dsal::iterator_adaptor<::std::istreambuf_iterator<char>>>);

