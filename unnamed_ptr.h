#pragma once

#include <memory>
#include <type_traits>
#include <functional>

namespace std
{
    class in_place_t {};

    constexpr in_place_t in_place;
}

namespace details
{
    template<class T, class Deleter>
    class deleter_with_pointer
    {
        T * ptr_;
        Deleter deleter_;

    public:
       static void destroy(void * data)
       {
          auto self = reinterpret_cast<deleter_with_pointer *>(data);
          self->deleter_(self->ptr_);
          delete self;
       }

       deleter_with_pointer(T * ptr, Deleter deleter)
            : ptr_(ptr)
            , deleter_(deleter)
       {}

       deleter_with_pointer            (deleter_with_pointer const &) = delete;
       deleter_with_pointer& operator =(deleter_with_pointer const &) = delete;
    };

    template<class T>
    class deleter_with_object
    {
       std::aligned_storage_t<sizeof(T), alignof(T)> store_;

       template<typename... CtorArgs>
       std::enable_if_t<std::is_constructible<T, CtorArgs...>::value> init(CtorArgs&&... args)
       {
          new(&store_) T(std::forward<CtorArgs>(args)...);
       }

       template<typename... CtorArgs>
       std::enable_if_t<!std::is_constructible<T, CtorArgs...>::value> init(CtorArgs&&... args)
       {
          new(&store_) T{ std::forward<CtorArgs>(args)... };
       }

    public:
       template<typename... CtorArgs>
       deleter_with_object(CtorArgs&&... args)
       {
          init(std::forward<CtorArgs>(args)...);
       }

       T * object_pointer()
       {
          return reinterpret_cast<T *>(&store_);
       }

       static void destroy(void * data)
       {
          auto self = reinterpret_cast<deleter_with_object *>(data);
          self->object_pointer()->~T();
          delete self;
       }

       deleter_with_object            (deleter_with_object const &) = delete;
       deleter_with_object& operator =(deleter_with_object const &) = delete;
    };

    template<class T, class Deleter>
    struct DeleterHolder : Deleter
    {
    private:
        T * ptr_;

   public:
        DeleterHolder(T * ptr, Deleter deleter)
            : Deleter(deleter)
            , ptr_(ptr)
        {
            static_assert(sizeof(DeleterHolder) == sizeof(void *), "sizeof(DeleterHolder) must be equal sizeof(void*)");
        }

        static void apply(void * data)
        {
            auto deleter = reinterpret_cast<DeleterHolder &>(data);
            deleter(deleter.ptr_);
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
    using deleter_func_t = void (*)(deleter_data_t);

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
       : deleter_func_(details::DeleterHolder<U, Deleter>::apply)
       , ptr_(ptr)
    {
       new(&deleter_data_) details::DeleterHolder<U, Deleter>(ptr, deleter);
    }

    template<class U, class Deleter>
    unnamed_ptr(U * ptr, Deleter deleter, is_convertible_tag<U> = nullptr, std::enable_if_t<!std::is_empty_v<Deleter>>* = nullptr) noexcept(is_nothrow_constructible<Deleter>)
        : deleter_func_(details::deleter_with_pointer<U, Deleter>::destroy)
        , deleter_data_(new details::deleter_with_pointer<U, Deleter>(ptr, std::move(deleter)))
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

    template<typename... CtorArgs>
    unnamed_ptr(std::in_place_t, CtorArgs&&... args) noexcept (std::is_nothrow_constructible_v<T, CtorArgs&&...>)
        : deleter_func_(details::deleter_with_object<T>::destroy)
        , deleter_data_(new details::deleter_with_object<T>(std::forward<CtorArgs>(args)...))
        , ptr_(reinterpret_cast<details::deleter_with_object<T> *>(deleter_data_)->object_pointer())
    {}

    template<class U>
    unnamed_ptr(unnamed_ptr<U> && other, is_convertible_tag<U> = nullptr) noexcept
        : unnamed_ptr(static_cast<T *>(other.get()), std::move(other))
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
};

template<class T, typename... CtorArgs>
unnamed_ptr<T> make_unnamed(CtorArgs&&... args)
{
    return unnamed_ptr<T>(std::in_place, std::forward<CtorArgs>(args)...); 
}
