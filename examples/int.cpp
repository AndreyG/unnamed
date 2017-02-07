#include "unnamed_ptr.h"

int main()
{
    unnamed_ptr<int> i1(new int(239));
    unnamed_ptr<int> i2(std::make_unique<int>(239));
    unnamed_ptr<int> i3 = make_unnamed<int>(239);
}

