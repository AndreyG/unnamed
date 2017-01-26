#include "unnamed_ptr.h"

#include <iostream>

class Base
{
    char bfield[16];
};

class Trash
{
public:
    char tfield[16];
};

class Derived : public Trash, public Base
{
public:
    ~Derived()
    {
        std::cout << "~Derived()\n";
    }
};

int main()
{
    unnamed_ptr<Base> b1 = make_unnamed<Derived>();
    unnamed_ptr<Base> b2 = std::make_unique<Derived>();
    b2 = std::make_unique<Derived>();
    auto d3 = std::make_unique<Derived>(); 
    unnamed_ptr<Base> b3 = std::move(d3);
    unnamed_ptr<void> b4 = std::move(b3);
}
