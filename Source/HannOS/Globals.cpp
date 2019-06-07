#include <algorithm>

using Constructor = void(*)();
using Destructor  = void(*)();

extern "C" Constructor constructorsStart;
extern "C" Constructor constructorsEnd;

extern "C" void doConstructors() {
  std::for_each(&constructorsStart, &constructorsEnd, [](Constructor c){
    (*c)();
  });
}

extern "C" Destructor destructorsStart;
extern "C" Destructor destructorsEnd;

extern "C" void doDestructors() {
  std::for_each(&destructorsStart, &destructorsEnd, [](Destructor d){
    (*d)();
  });
}
