#pragma once

#include <memory>
#include <type_traits>

namespace details
{
    template<typename T, typename... CtorArgs>
    std::enable_if_t<std::is_constructible<T, CtorArgs...>::value, T*> make(CtorArgs&&... args)
    {
        return new T(std::forward<CtorArgs>(args)...);
    }

    template<typename T, typename... CtorArgs>
    std::enable_if_t<!std::is_constructible<T, CtorArgs...>::value, T*> make(CtorArgs&&... args)
    {
        return new T{ std::forward<CtorArgs>(args)... };
    }

    template<class T, class Deleter>
    class deleter_holder
    {
        T * ptr_;
        Deleter deleter_;

    public:
        static void destroy(void * data)
        {
            auto self = reinterpret_cast<deleter_holder *>(data);
            self->deleter_(self->ptr_);
            delete self;
        }

        deleter_holder(T * ptr, Deleter deleter)
            : ptr_(ptr)
            , deleter_(deleter)
        {}

        deleter_holder            (deleter_holder const &) = delete;
        deleter_holder& operator =(deleter_holder const &) = delete;
    };

    template<class T, class Deleter>
    struct empty_deleter_holder : Deleter
    {
    private:
        T * ptr_;

    public:
        static void destroy(void * data)
        {
            auto deleter = reinterpret_cast<empty_deleter_holder &>(data);
            deleter(deleter.ptr_);
        }

        empty_deleter_holder(T * ptr, Deleter deleter)
            : Deleter(deleter)
            , ptr_(ptr)
        {
            static_assert(sizeof(empty_deleter_holder) == sizeof(void *),
                          "sizeof(empty_deleter_holder) must be equal to sizeof(void*)");
        }
    };
}

class type_erased_deleter
{
	using deleter_data_t = void *;
	using deleter_func_t = void (*)(deleter_data_t);

	deleter_func_t deleter_func_;
	deleter_data_t deleter_data_;

public:
	void operator() (void *)
	{
		deleter_func_(deleter_data_);
	}

	type_erased_deleter()
		: deleter_func_([] (void *) {})
	{}

	template<class U, class Deleter>
	type_erased_deleter(U * ptr, Deleter deleter, std::enable_if_t<std::is_empty<Deleter>::value>* = nullptr)
		: deleter_func_(details::empty_deleter_holder<U, Deleter>::destroy)
	{
		new(&deleter_data_) details::empty_deleter_holder<U, Deleter>(ptr, deleter);
	}

	template<class U, class Deleter>
	type_erased_deleter(U * ptr, Deleter deleter, std::enable_if_t<!std::is_empty<Deleter>::value>* = nullptr)
		: deleter_func_(details::deleter_holder<U, Deleter>::destroy)
		, deleter_data_(new details::deleter_holder<U, Deleter>(ptr, std::move(deleter)))
	{}
};

template<class T>
class unnamed_ptr : public std::unique_ptr<T, type_erased_deleter>
{
    template<class U>
    using is_convertible_tag = std::enable_if_t<std::is_convertible<U *, T *>::value> *;

    template<class U>
    friend class unnamed_ptr;

	typedef std::unique_ptr<T, type_erased_deleter> base_t;

public:
    template<class U, class Deleter>
    unnamed_ptr(U * ptr, Deleter deleter, is_convertible_tag<U> = nullptr) noexcept(std::is_nothrow_move_constructible<Deleter>::value)
		: base_t(ptr, type_erased_deleter(ptr, deleter))
    {
    }

    template<class U>
    unnamed_ptr(T * ptr, unnamed_ptr<U> && other) noexcept
        : base_t(ptr, other.get_deleter())
    {
		other.release();
    }

    template<class U>
    explicit unnamed_ptr(U * ptr, is_convertible_tag<U> = nullptr) noexcept
        : unnamed_ptr(ptr, std::default_delete<U>())
    {}

    template<class U, class Deleter>
    unnamed_ptr(std::unique_ptr<U, Deleter> && other, is_convertible_tag<U> = nullptr) noexcept(std::is_nothrow_move_constructible<Deleter>::value)
        : unnamed_ptr(other.release(), std::move(other.get_deleter()))
    {}

    unnamed_ptr(std::nullptr_t = nullptr) noexcept
        : base_t(nullptr)
    {}
};

template<class T, typename... CtorArgs>
unnamed_ptr<T> make_unnamed(CtorArgs&&... args)
{
    return unnamed_ptr<T>(details::make<T>(std::forward<CtorArgs>(args)...));
}
