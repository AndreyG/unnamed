#include "unnamed_ptr.h"

#include <iostream>
#include <functional>

class SelfDestroyer
{
    ~SelfDestroyer()
    {
        std::cout << "~SelfDestroyer()\n";
    }

public:
    void destroy()
    {
        delete this;
    }
};

int main()
{
    using namespace std::placeholders;

	auto deleter = [] (SelfDestroyer * sd) { sd->destroy(); };

    unnamed_ptr<SelfDestroyer> sd1(new SelfDestroyer, deleter);
    unnamed_ptr<SelfDestroyer> sd2(new SelfDestroyer, +static_cast<void (*)(SelfDestroyer *)>(deleter));
    unnamed_ptr<SelfDestroyer> sd3(new SelfDestroyer, std::bind(&SelfDestroyer::destroy, _1));
}
