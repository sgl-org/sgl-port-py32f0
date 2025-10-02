#include <errno.h>
#include <stdint.h>

extern uint8_t _end[];
extern uint8_t _estack[];
extern uint8_t _Min_Stack_Size[];
static void *__sbrk_heap_end;

void *_sbrk(ptrdiff_t incr)
{
    void *block, *end;
    uintptr_t limit;

    limit = (uintptr_t)&_estack - (uintptr_t)&_Min_Stack_Size;
    end = (void *)limit;

    if (!__sbrk_heap_end)
        __sbrk_heap_end = &_end;

    if (__sbrk_heap_end + incr > end) {
        errno = ENOMEM;
        return (void *)-1;
    }

    block = __sbrk_heap_end;
    __sbrk_heap_end += incr;

    return block;
}
