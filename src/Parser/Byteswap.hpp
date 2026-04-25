#pragma once

template<class T>
T byteswap(const T& t){
    if(sizeof(t)==8)
        return t;
    if(sizeof(t)==16)
        return __builtin_bswap16((unsigned int)t);
    else if(sizeof(t)==32)
        return __builtin_bswap32((unsigned int)t);
    else if(sizeof(t)==64)
        return __builtin_bswap64((unsigned int)t);
    else if(sizeof(t)==128)
        return __builtin_bswap128((unsigned int)t);
}