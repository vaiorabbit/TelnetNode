# TelnetNode #

A demonstration program showing how to transfer texts via socket API.

*   Created: 2012-04-29 (Sun)
*   Last Modified: 2012-04-29 (Sun)

## Usage ##

    <Terminal 1>                                <Terminal 2> 
    $ make all
    $ sudo ./server
    Password:
    Server started.                             $ sudo ./client
    Server got message. : Hello, Mr.Server.     Client started.
    Server got message. : Hello, Mr.Server.     ...
    Server got message. : Hello, Mr.Server.
    Server got message. : Hello, Mr.Server.
    Server got message. : Hello, Mr.Server.
    Server got message. : Hello, Mr.Server.
    Server got message. : Hello, Mr.Server.
    Server got message. : Hello, Mr.Server.
    Server got message. : Hello, Mr.Server.
    Server got message. : Hello, Mr.Server.
    Server got message. : bye                   Client got message. : bye
    Server terminated.                          Client terminated.

*   See TelnetNode class declaration for user API.
*   Note: On UNIX platforms, the use of port 0~1023 needs superuser privilege.

## Reference ##

TelnetNode.h is created through refactoring the snippet provided by John Ratcliff.

*   John Ratcliff's telnet code snippet
	*   http://code.google.com/p/telnet/
	*   http://codesuppository.blogspot.jp/2009/05/telnet-code-snippet-that-embeds-telnet.html

## License ##

The MIT License

Copyright (c) 2012 vaiorabbit ( http://twitter.com/vaiorabbit )

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

<!--
Local Variables:
mode: markdown
coding: utf-8-unix
End:
-->
