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
            static_assert(sizeof(empty_deleter_holder) == sizeof(void *), "sizeof(empty_deleter_holder) must be equal sizeof(void*)");
        }
    };
}

template<class T>
class unnamed_ptr
{
    template<class U>
    static constexpr bool is_convertible = std::is_convertible<U *, T *>::value;

    template<class U>
    using is_convertible_tag = std::enable_if_t<is_convertible<U>> *;

    template<class U>
    using ref_to_self = std::enable_if_t<is_convertible<U>, unnamed_ptr>&;

    using deleter_data_t = void *;
    using deleter_func_t = void(*)(deleter_data_t);

    template<class Deleter>
    static constexpr bool is_nothrow_constructible
        = std::is_nothrow_constructible_v<std::remove_cv_t<std::remove_reference_t<Deleter>>, Deleter &&>;

    template<class U>
    friend class unnamed_ptr;

private:
    deleter_func_t deleter_func_;
    deleter_data_t deleter_data_;
    T * ptr_;

public:
    template<class U, class Deleter>
    unnamed_ptr(U * ptr, Deleter deleter, is_convertible_tag<U> = nullptr, std::enable_if_t<std::is_empty_v<Deleter>>* = nullptr) noexcept
        : deleter_func_(details::empty_deleter_holder<U, Deleter>::destroy)
        , ptr_(ptr)
    {
        new(&deleter_data_) details::empty_deleter_holder<U, Deleter>(ptr, deleter);
    }

    template<class U, class Deleter>
    unnamed_ptr(U * ptr, Deleter deleter, is_convertible_tag<U> = nullptr, std::enable_if_t<!std::is_empty_v<Deleter>>* = nullptr) noexcept(is_nothrow_constructible<Deleter>)
        : deleter_func_(details::deleter_holder<U, Deleter>::destroy)
        , deleter_data_(new details::deleter_holder<U, Deleter>(ptr, std::move(deleter)))
        , ptr_(ptr)
    {
    }

    template<class U>
    unnamed_ptr(T * ptr, unnamed_ptr<U> && other) noexcept
        : deleter_func_(other.deleter_func_)
        , deleter_data_(other.deleter_data_)
        , ptr_(ptr)
    {
        other.ptr_ = nullptr;
    }

    template<class U>
    explicit unnamed_ptr(U * ptr, is_convertible_tag<U> = nullptr) noexcept
        : unnamed_ptr(ptr, std::default_delete<U>())
    {}

    template<class U, class Deleter>
    unnamed_ptr(std::unique_ptr<U, Deleter> && ptr, is_convertible_tag<U> = nullptr) noexcept(is_nothrow_constructible<Deleter>)
        : unnamed_ptr(ptr.release(), std::move(ptr.get_deleter()))
    {}

    template<class U>
    unnamed_ptr(unnamed_ptr<U> && other, is_convertible_tag<U> = nullptr) noexcept
        : unnamed_ptr(static_cast<T *>(other.get()), std::move(other))
    {}

    unnamed_ptr(std::nullptr_t) noexcept
        : ptr_(nullptr)
    {}

    template<class U>
    ref_to_self<U> operator = (unnamed_ptr<U> && other) noexcept
    {
        std::swap(deleter_func_, other.deleter_func_);
        std::swap(deleter_data_, other.deleter_data_);
        std::swap(ptr_, other.ptr_);
        return *this;
    }

    template<class U, class D>
    ref_to_self<U> operator = (std::unique_ptr<U, D> && other)
    {
        return *this = unnamed_ptr(std::move(other));
    }

    ~unnamed_ptr()
    {
        if (ptr_)
            deleter_func_(deleter_data_);
    }

    T* get() const noexcept
    {
        return ptr_;
    }

    T* operator -> () const noexcept
    {
        return get();
    }

    explicit operator bool() const
    {
        return ptr_ != nullptr;
    }
};

template<class T, typename... CtorArgs>
unnamed_ptr<T> make_unnamed(CtorArgs&&... args)
{
    return unnamed_ptr<T>(details::make<T>(std::forward<CtorArgs>(args)...));
}
