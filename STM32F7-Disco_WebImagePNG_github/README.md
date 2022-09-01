
# PNG image download from a web server

The ethernet started working by disabling the cache. I have inserted these two defines in the "variant.h" file:

    #define D_CACHE_DISABLED
    #define I_CACHE_DISABLED

This project have a problem: when I use the .PNG decoder the ethernet stop to work.
