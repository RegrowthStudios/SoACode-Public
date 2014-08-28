#pragma once
// C#-Style Properties

// Special Setter Function
#define PROP_FSET(TYPE, NAME) \
    __declspec(property( put = __propput__##NAME )) TYPE NAME; \
    void __propput__##NAME ( TYPE value )

// Special Getter Function
#define PROP_FGET(TYPE, NAME) \
    __declspec(property( get = __propget__##NAME )) TYPE NAME; \
    TYPE __propget__##NAME ()
