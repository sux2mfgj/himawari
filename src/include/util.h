#pragma once

#define align(x, a) __align_mask(x, (typeof(x))(a)-1)
#define __align_mask(x,mask) (((x)+(mask))&~(mask))

#define max(a, b)               \
    ({                          \
	__typeof__(a) _a = (a); \
	__typeof__(b) _b = (b); \
	_a > _b ? _a : _b;      \
    })


#define round_up(x,y) (((x) + (y) - 1) & ~((y)-1))
#define round_down(x,y) ((x) & ~((y)-1))

