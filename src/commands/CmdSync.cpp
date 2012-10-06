////////////////////////////////////////////////////////////////////////////////
// taskwarrior - a command line task list manager.
//
// Copyright 2006-2012, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#define L10N                                           // Localization complete.

#include <iostream>
#include <inttypes.h>
#include <Context.h>
#include <Socket.h>
#include <text.h>
#include <i18n.h>
#include <CmdSync.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdSync::CmdSync ()
{
  _keyword     = "synchronize";
  _usage       = "task          synchronize";
  _description = STRING_CMD_SYNC_USAGE;
  _read_only   = false;
  _displays_id = true;
}

////////////////////////////////////////////////////////////////////////////////
int CmdSync::execute (std::string& output)
{
  int status = 0;

  // If no server is set up, quit.
  std::string connection = context.config.get ("taskd.server");
  if (connection == "" ||
      connection.find (':') == std::string::npos)
    throw std::string (STRING_CMD_SYNC_NO_SERVER);

  // Obtain credentials.
  std::string credentials = context.config.get ("taskd.credentials");

  // Read backlog.data.
  std::string payload = "";
  File backlog (context.config.get ("data.location") + "/backlog.data");
  if (backlog.exists ())
    backlog.read (payload);

  // Send 'sync' + payload.
  Msg request, response;
  request.set ("type", "sync");
  // TODO Add the other header fields.

  request.setPayload (payload);
  std::cout << "# request:\n"
            << request.serialize ();

  context.debug ("sync with " + connection);
  if (send (connection, request, response))
  {
    std::cout << "# response:\n"
              << response.serialize ();
    if (response.get ("code") == "200")
    {
      payload = response.getPayload ();
      std::vector <std::string> lines;
      split (lines, payload, '\n');

      std::string synch_key;
      std::vector <std::string>::iterator line;
      for (line = lines.begin (); line != lines.end (); ++line)
      {
        if ((*line)[0] == '[')
        {
          // TODO Apply tasks.
          std::cout << "# task: " << *line << "\n";
        }
        else
        {
          synch_key = *line;
          context.debug ("Synch key " + synch_key);
        }
      }

      // TODO Truncate backlog.data.
      // TODO Store new synch key.
    }
    else
    {
      context.error ("Task Server problem.");
      status = 2;
    }

    // TODO Display all errors returned.
  }
  else
  {
    context.error ("Could not connect to Task Server.");
    status = 1;
  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
bool CmdSync::send (
  const std::string& to,
  const Msg& request,
  Msg& response)
{
  std::string::size_type colon = to.find (':');
  if (colon == std::string::npos)
    throw std::string ("ERROR: Malformed configuration setting '") + to + "'";

  std::string server = to.substr (0, colon);
  int port = strtoimax (to.substr (colon + 1).c_str (), NULL, 10);

  try
  {
    Socket s (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    s.connect (server, port);
    s.write (request.serialize () + "\r\n");

    std::string incoming;
    s.read (incoming);
    s.close ();

    response.parse (incoming);

    // Indicate message sent.
    context.debug ("sync tx complete");
    return true;
  }

  catch (std::string& error)
  {
    context.error (error);
  }

  // Indicate message failed.
  return false;
}

////////////////////////////////////////////////////////////////////////////////
