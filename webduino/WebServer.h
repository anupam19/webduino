/* 
Basic Arduino web server
Copyright 2009 Ben Combee

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <Ethernet.h>
#include <string.h>
#include <stdlib.h>

// utility macros
#define CRLF "\r\n"
#define P(name)   static const prog_uchar name[] PROGMEM
#define SIZE(array) (sizeof(array) / sizeof(*array))

class WebServer: public Server
{
public:
  enum ConnectionType { INVALID, GET, HEAD, POST };
  typedef void Command(WebServer &server, ConnectionType type);

  WebServer(const char *urlPrefix = "/", int port = 80);
  void begin();
  void processConnection();

  void setDefaultCommand(Command *cmd);
  void setFailureCommand(Command *cmd);
  void addCommand(const char *verb, Command *cmd);

  void printCRLF();
  void printP(const prog_uchar *str);

  void radioButton(const char *name, const char *val, const char *label, bool selected);
  void checkBox(const char *name, const char *val, const char *label, bool selected);

  // returns next character or -1 if we're at end-of-stream
  int read();

  // put a character that's been read back into the input pool
  void push(char ch);
  
  // returns true if the string is next in the stream.  Doesn't consume any character if false,
  // so can be used to try out different expected values.
  bool expect(const char *expectedStr);
  
  // returns true if we're not at end-of-stream
  bool readURLParam(char *name, int nameLen, char *value, int valueLen);

  void httpFail();
  void httpSuccess(bool forceRefresh = false, const char *contentType = "text/html");
  void httpSeeOther(const char *otherURL);

  static void defaultFailCmd(WebServer &server, ConnectionType type);

private:
  Client *m_client;
  const char *m_urlPrefix;

  char m_pushback[32];
  char m_pushbackDepth;

  Command *m_failureCmd;
  Command *m_defaultCmd;
  struct CommandMap
  {
    const char *verb;
    Command *cmd;
  } m_commands[8];
  char m_cmdCount;

  void reset();
  void getRequest(WebServer::ConnectionType &type, char *request, int length);
  bool dispatchCommand(ConnectionType requestType, const char *verb);
  void skipHeaders();
};

WebServer::WebServer(const char *urlPrefix, int port) :
  Server(port), 
  m_client(0),
  m_urlPrefix(urlPrefix), 
  m_pushbackDepth(0),
  m_cmdCount(0), 
  m_failureCmd(&defaultFailCmd), 
  m_defaultCmd(&defaultFailCmd)
{
}

void WebServer::begin()
{
  Server::begin();
}

void WebServer::setDefaultCommand(Command *cmd)
{
  m_defaultCmd = cmd;
}

void WebServer::setFailureCommand(Command *cmd)
{
  m_failureCmd = cmd;
}

void WebServer::addCommand(const char *verb, Command *cmd)
{
  if (m_cmdCount < SIZE(m_commands))
  {
    m_commands[m_cmdCount].verb = verb;
    m_commands[m_cmdCount++].cmd = cmd;
  }
}

void WebServer::printP(const prog_uchar *str)
{
  char ch;
  while ((ch = pgm_read_byte(str++)))
    print(ch, BYTE);
}

void WebServer::printCRLF()
{
  print('\r', BYTE);
  print('\n', BYTE);
}

bool WebServer::dispatchCommand(ConnectionType requestType, const char *verb)
{
  if (verb[0] == 0)
  {
    m_defaultCmd(*this, requestType);
    return true;
  }
  
  if (verb[0] == '/')
  {
    if (verb[1] == 0)
    {
      m_defaultCmd(*this, requestType);
      return true;
    }
    else
    {
      char i;
      for (i = 0; i < m_cmdCount; ++i)
      {
        if (strcmp(verb + 1, m_commands[i].verb) == 0)
        {
          m_commands[i].cmd(*this, requestType);
          return true;
        }  
      }     
    }
  }    
  return false;
}

void WebServer::processConnection()
{
  Client client = available();
  if (client) {
    m_client = &client;

    static char request[32];
    request[0] = 0;
    ConnectionType requestType = INVALID; 
    getRequest(requestType, request, 32);
    skipHeaders();
    
    int urlPrefixLen = strlen(m_urlPrefix);
    if (requestType == INVALID ||
        strncmp(request, m_urlPrefix, urlPrefixLen) != 0 ||
        !dispatchCommand(requestType, request + urlPrefixLen))
    {
       m_failureCmd(*this, requestType);
    }

    client.stop();
    m_client = NULL;
  }
}

void WebServer::httpFail()
{
  P(failMsg) = 
    "HTTP/1.0 400 Bad Request" CRLF
    "Content-Type: text/html" CRLF
    CRLF
    "<h1>EPIC FAIL</h1>";

  printP(failMsg);
}

void WebServer::defaultFailCmd(WebServer &server, WebServer::ConnectionType type)
{
  server.httpFail();
}

void WebServer::httpSuccess(bool forceRefresh, const char *contentType)
{
  P(successMsg1) =
    "HTTP/1.0 200 OK" CRLF
    "Content-Type: ";

  P(successMsg2) =
    "; charset=UTF-8" CRLF
    "Pragma: no-cache" CRLF;

  P(successMsg3) =
    "Refresh: 1" CRLF;

  printP(successMsg1);
  print(contentType);
  printP(successMsg2);
  if (forceRefresh)
    printP(successMsg3);
  printCRLF();
}

void WebServer::httpSeeOther(const char *otherURL)
{
  P(seeOtherMsg) =
    "HTTP/1.0 303 See Other" CRLF
    "Location: ";

  printP(seeOtherMsg);
  print(otherURL);
  printCRLF();
  printCRLF();
}

int WebServer::read()
{
  if (m_pushbackDepth == 0)
    return m_client->read();
  else
    return m_pushback[--m_pushbackDepth];
}

void WebServer::push(char ch)
{
  m_pushback[m_pushbackDepth++] = ch;
  // can't raise error here, so just replace last char over and over
  if (m_pushbackDepth == SIZE(m_pushback))
    m_pushbackDepth = SIZE(m_pushback) - 1;  
}

void WebServer::reset()
{
  m_pushbackDepth = 0;
}

bool WebServer::expect(const char *str)
{
  const char *curr = str;
  while (*curr != 0)
  {
    int ch = read();
    if (ch != *curr++)
    {
      // push back ch and the characters we accepted
      if (ch != -1)
        push(ch);
      while (--curr != str)
        push(curr[-1]);
      return false;
    }
  }
  return true;
}

bool WebServer::readURLParam(char *name, int nameLen, char *value, int valueLen)
{
  // assume name is at current place in stream
  int ch;

  // clear out name and value so they'll be NUL terminated
  memset(name, 0, nameLen);
  memset(value, 0, valueLen);
  
  while ((ch = read()) != -1)
  {
    if (ch == '+')
    {
      ch = ' ';
    }
    else if (ch == '=')
    {
      /* that's end of name, so switch to storing in value */
      nameLen = 0;
      continue;
    }
    else if (ch == '&')
    {
      /* that's end of pair, go away */
      return true;
    }
    else if (ch == '%')
    {
      /* handle URL encoded characters by converting back to original form */
      int ch1 = read();
      int ch2 = read();
      if (ch1 == -1 || ch2 == -1)
         return false;
      char hex[3] = { ch1, ch2, 0 };
      ch = strtoul(hex, NULL, 16);
    }

    // check against 1 so we don't overwrite the final NUL
    if (nameLen > 1)
    {
      *name++ = ch;
      --nameLen;
    }
    else if (valueLen > 1)
    {
      *value++ = ch;
      --valueLen;
    }
  }
  return (ch != -1);
}
  
void WebServer::getRequest(WebServer::ConnectionType &type, char *request, int length)
{
  --length; // save room for NUL
  
  type = INVALID;

  // store the GET/POST line of the request
  if (expect("GET "))
      type = GET;
  else if (expect("HEAD "))
      type = HEAD;
  else if (expect("POST "))
      type = POST;

  // if it doesn't start with any of those, we have an unknown method so just eat rest of header 

  int ch;
  while ((ch = read()) != -1)
  {
    // stop storing at first space or end of line
    if (ch == ' ' || ch == '\n' || ch == '\r')
    {
      length = 0;
      break;
    }
    if (length > 0)
    {
      *request = ch;
      ++request;
      --length;
    }
  }
  // NUL terminate
  *request = 0;
}

void WebServer::skipHeaders()
{
  // look for the CRLFCRLF at the end of the headers, read characters until then
  // store the GET/POST line of the request
  char state = 0;
  int ch; 
  while ((ch = read()) != -1)
  {
    if (ch == '\r')
    {
      if (state == 0) state = 1; 
      else if (state == 2) state = 3;
    }
    else if (ch == '\n') 
    { 
      if (state == 1) state = 2; 
      else if (state == 3) return; 
    }
    else
    {
      // any other character resets the search state
      state = 0;
    }
  }
}

void WebServer::checkBox(const char *name, const char *val, const char *label, bool selected)
{
  print("<label><input type='checkbox' name='");
  print(name);
  print("' value='");
  print(val);
  print("' ");
  if (selected)
    print("checked ");
  print("/> ");
  print(label);
  print("</label>");
}

void WebServer::radioButton(const char *name, const char *val, const char *label, bool selected)
{
  print("<label><input type='radio' name='");
  print(name);
  print("' value='");
  print(val);
  print("' ");
  if (selected)
    print("checked ");
  print("/> ");
  print(label);
  print("</label>");
}