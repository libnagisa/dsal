#include <nagisa/dsal/iterator/iterator.h>

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

