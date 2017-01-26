#include "unnamed_ptr.h"

#include <boost/test/unit_test.hpp>
#include <boost/noncopyable.hpp>
#include <boost/checked_delete.hpp>
#include <boost/logic/tribool.hpp>

namespace
{
   struct S : boost::noncopyable
   {
   private:
      boost::logic::tribool & is_alive_;
      char trash[16];

   public:
      S(boost::logic::tribool & is_alive)
         : is_alive_(is_alive)
      {
         is_alive_ = true;
      }

      ~S()
      {
         BOOST_CHECK_EQUAL(is_alive_, true);
         is_alive_ = false;
      }

      int field;
   };

   using SPtr     = unnamed_ptr<S>;
   using IntPtr   = unnamed_ptr<int>;
}

BOOST_AUTO_TEST_SUITE(Borrowed)

BOOST_AUTO_TEST_CASE(DirectConstruction)
{
   boost::logic::tribool object_state = boost::logic::indeterminate;
   {
      SPtr s(new S(object_state));
      BOOST_CHECK(object_state == true);
      auto i_ptr = &s->field;
      BOOST_CHECK(i_ptr != static_cast<void *>(s.get()));
      IntPtr i(i_ptr, std::move(s));
      BOOST_CHECK(object_state == true);
   }
   BOOST_CHECK(object_state == false);
}

BOOST_AUTO_TEST_CASE(InPlace)
{
   boost::logic::tribool object_state = boost::logic::indeterminate;
   {
      auto s = make_unnamed<S>(object_state);
      BOOST_CHECK(object_state == true);
      auto i_ptr = &s->field;
      BOOST_CHECK(i_ptr != static_cast<void *>(s.get()));
      IntPtr i(i_ptr, std::move(s));
      BOOST_CHECK(object_state == true);
   }
   BOOST_CHECK(object_state == false);
}

BOOST_AUTO_TEST_CASE(DirectConstructionWithCustomDeleter)
{
   boost::logic::tribool object_state = boost::logic::indeterminate;
   {
      unnamed_ptr<S> s(new S(object_state), ([&object_state] (S * s) {
         delete s;
         BOOST_CHECK(object_state == false);
         object_state = boost::logic::indeterminate;
      }));
      BOOST_CHECK(object_state == true);
      auto i_ptr = &s->field;
      BOOST_CHECK(i_ptr != static_cast<void *>(s.get()));
      IntPtr i(i_ptr, std::move(s));
      BOOST_CHECK(object_state == true);
   }
   BOOST_CHECK(boost::logic::indeterminate(object_state));
}

BOOST_AUTO_TEST_SUITE_END()