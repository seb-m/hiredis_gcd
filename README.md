# Description

This is an [hiredis](https://github.com/antirez/hiredis) async adapter to integrate with [GCD](https://developer.apple.com/library/ios/#documentation/Performance/Reference/GCD_libdispatch_Ref/Reference/reference.html) (Grand Central Dispatch library).

# Instructions

[Download hiredis](https://github.com/antirez/hiredis/downloads) and integrate the following files with the hiredis project

    libdispatch.c → hiredis/adapters/
    example-libdispatch.c → hiredis/

Then insert this new rule into hiredis' `Makefile`

    hiredis-example-libdispatch: example-libdispatch.c adapters/libdispatch.h $(STLIBNAME)
    	$(CC) -o $@ $(REAL_CFLAGS) $(REAL_LDFLAGS) example-libdispatch.c $(STLIBNAME)

Compile and run the basic example `example-libdispatch.c`

    $ make hiredis-example-libdispatch
    $ ./hiredis-example-libdispatch foo
    Connected...
    argv[end-1]: foo
    Disconnected...


# MIT License

    Copyright (c) 2012 Sébastien Martini <seb@dbzteam.org>

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
