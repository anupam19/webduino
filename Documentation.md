# How Do I Use This? #

Put a copy of WebServer.h in the folder with your sketch file.  In your sketch, add the line

> `#include "WebServer.h"`

at the top of the file.

In your code, declare a `WebServer` object, be sure to call the `begin()` method in your setup code, and in the main loop, call the `processConnection()` method.

# Types #

  * WebServer::ConnectionType
> > INVALID, GET, HEAD, POST

  * WebServer::Command
> > typedef void Command(WebServer &server, ConnectionType type, char **urlTail, bool tailComplete)**

# Methods #

  * WebServer(urlPrefix, int port)
  * begin()
  * processConnection(buffer, bufferLen)
  * processConnection()

  * setDefaultCommand(cmd)
  * setFailureCommand(cmd)
  * addCommand(verb, cmd)

  * print(...)
  * printCRLF()
  * printP(progStr)

  * write(data, length)
  * writeP(progData, length)

  * readPOSTparam(name, nameLen, value, valueLen)
  * nextURLparam(tail, name, nameLen, value, valueLen)

  * radioButton(name, val, label, selected)
  * checkBox(name, val, label, selected)

  * int read()
  * push(ch)
  * bool expect(expectedStr)

  * httpFail()
  * httpSuccess(contentType, extraHeaders)
  * httpSeeOther(otherURL)