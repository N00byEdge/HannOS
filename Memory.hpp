#include<cstdint>

// You get a 0x1000/4k chunk no matter what you do. Have fun!
void *kMalloc();
void kFree(void *ptr);
void *kCalloc();

void *kMalloc(intptr_t sz);
void *kFree(void *ptr, intptr_t sz);
void *kCalloc(intptr_t sz, intptr_t num);

