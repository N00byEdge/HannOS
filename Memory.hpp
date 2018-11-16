#include<cstdint>

// You get a 0x1000/4k chunk no matter what you do. Have fun!
void *kMalloc();
void kFree(void *ptr);
void *kCalloc();

// Returns the memory allocated, in bytes
std::intptr_t allocated();
