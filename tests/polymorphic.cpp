#include "unnamed_ptr.h"

#include <boost/test/unit_test.hpp>
#include <boost/noncopyable.hpp>
#include <boost/checked_delete.hpp>
#include <boost/logic/tribool.hpp>

namespace
{
   class Base : boost::noncopyable
   {
      char bfield[16];

   protected:
      ~Base() {}
   };

   using BasePtr = unnamed_ptr<Base>;

   class Trash
   {
   public:
      char tfield[16];
   };

   class Derived : public Trash, public Base
   {
      boost::logic::tribool & is_alive_;

   public:
      Derived(boost::logic::tribool & is_alive)
         : is_alive_(is_alive)
      {
         is_alive_ = true;
      }

      ~Derived()
      {
         BOOST_CHECK_EQUAL(is_alive_, true);
         is_alive_ = false;
      }
   };
}

BOOST_AUTO_TEST_SUITE(Inheritance)

BOOST_AUTO_TEST_CASE(DirectConstruction)
{
   boost::logic::tribool object_state = boost::logic::indeterminate;
   {
      BasePtr b(new Derived(object_state));
      BOOST_CHECK(object_state == true);
   }
   BOOST_CHECK(object_state == false);
}

BOOST_AUTO_TEST_CASE(InPlace)
{
   boost::logic::tribool object_state = boost::logic::indeterminate;
   {
      auto d = make_unnamed<Derived>(object_state);
      BOOST_CHECK(object_state == true);

      void * d_ptr = d.get();
      BasePtr b = std::move(d);
      void * b_ptr = b.get();
      BOOST_CHECK(d_ptr != b_ptr);
   }
   BOOST_CHECK(object_state == false);
}

BOOST_AUTO_TEST_CASE(FromUniquePointer)
{
   boost::logic::tribool object_state = boost::logic::indeterminate;
   {
      auto d = std::make_unique<Derived>(object_state);
      BOOST_CHECK(object_state == true);

      void * d_ptr = d.get();
      BasePtr b = std::move(d);
      void * b_ptr = b.get();
      BOOST_CHECK(d_ptr != b_ptr);
   }
   BOOST_CHECK(object_state == false);
}

BOOST_AUTO_TEST_CASE(FromUniquePointerWithCustomDeleter)
{
   boost::logic::tribool object_state = boost::logic::indeterminate;
   {
      auto deleter = [&object_state] (Derived * d) {
         delete d;
         BOOST_CHECK(object_state == false);
         object_state = boost::logic::indeterminate;
      };
      std::unique_ptr<Derived, decltype(deleter)> d(new Derived(object_state), deleter);
      BOOST_CHECK(object_state == true);

      void * d_ptr = d.get();
      BasePtr b = std::move(d);
      void * b_ptr = b.get();
      BOOST_CHECK(d_ptr != b_ptr);
   }
   BOOST_CHECK(boost::logic::indeterminate(object_state));
}

BOOST_AUTO_TEST_SUITE_END()