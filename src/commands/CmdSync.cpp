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
  _displays_id = false;
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
  std::string credentials_string = context.config.get ("taskd.credentials");
  if (credentials_string == "")
    throw std::string (STRING_CMD_SYNC_BAD_CRED);

  std::vector <std::string> credentials;
  split (credentials, credentials_string, "/");
  if (credentials.size () != 3)
    throw std::string (STRING_CMD_SYNC_BAD_CRED);

  // Read backlog.data.
  std::string payload = "";
  File backlog (context.config.get ("data.location") + "/backlog.data");
  if (backlog.exists ())
    backlog.read (payload);

  // Send 'sync' + payload.
  Msg request, response;
  request.set ("type", "sync");
  request.set ("org",  credentials[0]);
  request.set ("user", credentials[1]);
  request.set ("key",  credentials[2]);

  // TODO Add the other header fields.

  request.setPayload (payload);
  std::cout << "# request:\n"
            << request.serialize ();

  context.debug ("sync with " + connection);
  if (send (connection, request, response))
  {
    std::cout << "# response:\n"
              << response.serialize ();
    std::string code = response.get ("code");
    if (code == "200")
    {
      payload = response.getPayload ();
      std::vector <std::string> lines;
      split (lines, payload, '\n');
      std::cout << "# received " << lines.size () << " lines of data\n";

      // Load all tasks.
      std::vector <Task> all = context.tdb2.all_tasks ();

      std::string synch_key = "";
      std::vector <std::string>::iterator line;
      for (line = lines.begin (); line != lines.end (); ++line)
      {
        if ((*line)[0] == '[')
        {
          std::cout << "# task: " << *line << "\n";
          Task from_server (*line);
          std::cout << "  " << from_server.get ("uuid")
                    << " "  << from_server.get ("description")
                    << "\n";
          context.tdb2.modify (from_server);
        }
        else
        {
          synch_key = *line;
          context.debug ("Synch key " + synch_key);
        }
      }

      // Only update everything if there is a new synch_key.  No synch_key means
      // something horrible happened on the other end of the wire.
      if (synch_key != "")
      {
        // Truncate backlog.data, save new synch_key.
        context.tdb2.backlog.clear ();
        context.tdb2.backlog.add_line (synch_key + "\n");

        // Commit all changes.
        context.tdb2.commit ();
      }
    }
    else if (code == "430")
    {
      context.error ("Not authorized.  Could be incorrect credentials or "
                     "server account not enabled.");
      status = 2;
    }
    else
    {
      context.error ("Task Server error: " + code + " " + response.get ("status"));
      status = 2;
    }

    // Display all errors returned.  This is required by the server protocol.
    std::string to_be_displayed = response.get ("messages");
    if (to_be_displayed != "")
    {
      if (context.verbose ("footnote"))
        context.footnote (to_be_displayed);
      else
        context.debug (to_be_displayed);
    }
  }

  // Some kind of low-level error:
  //   - Server down
  //   - Wrong address
  //   - Wrong port
  //   - Firewall
  //   - Network error
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
