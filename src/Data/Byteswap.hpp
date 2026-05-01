#pragma once
#include <utility>

template<class T>
T byteswap(const T& t){
    if(sizeof(T)==1)
        return t;
    if(sizeof(T)==2)
        return __builtin_bswap16((unsigned int)t);
    else if(sizeof(T)==4)
        return __builtin_bswap32((unsigned int)t);
    else if(sizeof(T)==8)
        return __builtin_bswap64((unsigned int)t);
    assert(0 && "unreachable");
    return 0;
}