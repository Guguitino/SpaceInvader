#include "ConsoleGL.h"
#include "testmysqrt.h"

namespace test{
double sqrt(double number){
    return detail::mysqrt(number);
}
}
