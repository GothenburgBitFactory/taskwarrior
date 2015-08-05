////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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

#include <cmake.h>
#include <sstream>
#include <inttypes.h>
#include <signal.h>
#include <Context.h>
#include <Filter.h>
#include <Color.h>
#include <text.h>
#include <util.h>
#include <i18n.h>
#include <CmdSync.h>

extern Context context;

////////////////////////////////////////////////////////////////////////////////
CmdSync::CmdSync ()
{
  _keyword               = "synchronize";
  _usage                 = "task          synchronize [initialize]";
  _description           = STRING_CMD_SYNC_USAGE;
  _read_only             = false;
  _displays_id           = false;
  _needs_gc              = false;
  _uses_context          = false;
  _accepts_filter        = false;
  _accepts_modifications = false;
  _accepts_miscellaneous = true;
  _category              = Command::Category::migration;
}

////////////////////////////////////////////////////////////////////////////////
int CmdSync::execute (std::string& output)
{
  int status = 0;
#ifdef HAVE_LIBGNUTLS
  std::stringstream out;

  Filter filter;
  if (filter.hasFilter ())
    throw std::string (STRING_ERROR_NO_FILTER);

  // Loog for the 'init' keyword to indicate one-time pending.data upload.
  bool first_time_init = false;
  std::vector <std::string> words = context.cli2.getWords ();
  for (auto& word : words)
  {
    if (closeEnough ("initialize", word, 4))
    {
      if (!context.config.getBoolean ("confirmation") ||
          confirm (STRING_CMD_SYNC_INIT))
        first_time_init = true;
      else
        throw std::string (STRING_CMD_SYNC_NO_INIT);
    }
  }

  // If no server is set up, quit.
  std::string connection = context.config.get ("taskd.server");
  if (connection == "" ||
      connection.rfind (':') == std::string::npos)
    throw std::string (STRING_CMD_SYNC_NO_SERVER);

  // Obtain credentials.
  std::string credentials_string = context.config.get ("taskd.credentials");
  if (credentials_string == "")
    throw std::string (STRING_CMD_SYNC_BAD_CRED);

  std::vector <std::string> credentials;
  split (credentials, credentials_string, "/");
  if (credentials.size () != 3)
    throw std::string (STRING_CMD_SYNC_BAD_CRED);

  // This was a Boolean value in 2.3.0, and is a tri-state since 2.4.0.
  std::string trust_value = context.config.get ("taskd.trust");
  if (trust_value != "strict" &&
      trust_value != "ignore hostname" &&
      trust_value != "allow all")
    throw std::string (STRING_CMD_SYNC_TRUST_OBS);

  enum TLSClient::trust_level trust = TLSClient::strict;
  if (trust_value  == "allow all")
    trust = TLSClient::allow_all;
  else if (trust_value == "ignore hostname")
    trust = TLSClient::ignore_hostname;

  // CA must exist, if provided.
  File ca (context.config.get ("taskd.ca"));
  if (ca._data != "" && ! ca.exists ())
    throw std::string (STRING_CMD_SYNC_BAD_CA);

  if (trust == TLSClient::allow_all && ca._data != "")
    throw std::string (STRING_CMD_SYNC_TRUST_CA);

  File certificate (context.config.get ("taskd.certificate"));
  if (! certificate.exists ())
    throw std::string (STRING_CMD_SYNC_BAD_CERT);

  File key (context.config.get ("taskd.key"));
  if (! key.exists ())
    throw std::string (STRING_CMD_SYNC_BAD_KEY);

  // If this is a first-time initialization, send pending.data, not
  // backlog.data.
  std::string payload = "";
  int upload_count = 0;
  if (first_time_init)
  {
    // Delete backlog.data.  Because if we're uploading everything, the list of
    // deltas is meaningless.
    context.tdb2.backlog._file.truncate ();

    auto pending = context.tdb2.pending.get_tasks ();
    for (auto& i : pending)
    {
      payload += i.composeJSON () + "\n";
      ++upload_count;
    }
  }
  else
  {
    std::vector <std::string> lines = context.tdb2.backlog.get_lines ();
    for (auto& i : lines)
    {
      if (i[0] == '{')
        ++upload_count;

      payload += i + "\n";
    }
  }

  // Send 'sync' + payload.
  Msg request;
  request.set ("protocol", "v1");
  request.set ("type",     "sync");
  request.set ("org",      credentials[0]);
  request.set ("user",     credentials[1]);
  request.set ("key",      credentials[2]);

  request.setPayload (payload);

  if (context.verbose ("sync"))
    out << format (STRING_CMD_SYNC_PROGRESS, connection)
        << "\n";

  // Ignore harmful signals.
  signal (SIGHUP,    SIG_IGN);
  signal (SIGINT,    SIG_IGN);
  signal (SIGPIPE,   SIG_IGN);
  signal (SIGTERM,   SIG_IGN);
  signal (SIGUSR1,   SIG_IGN);
  signal (SIGUSR2,   SIG_IGN);

  Msg response;
  if (send (connection, ca._data, certificate._data, key._data, trust, request, response))
  {
    std::string code = response.get ("code");
    if (code == "200")
    {
      Color colorAdded    (context.config.get ("color.sync.added"));
      Color colorChanged  (context.config.get ("color.sync.changed"));

      int download_count = 0;
      payload = response.getPayload ();
      std::vector <std::string> lines;
      split (lines, payload, '\n');

      // Load all tasks, but only if necessary.  There is always a sync key in
      // the payload, so if there are two or more lines, then we have merging
      // to perform, otherwise it's just a backlog.data update.
      if (lines.size () > 1)
        context.tdb2.all_tasks ();

      std::string sync_key = "";
      for (auto& line : lines)
      {
        if (line[0] == '{')
        {
          ++download_count;

          Task from_server (line);
          std::string uuid = from_server.get ("uuid");

          // Is it a new task from the server, or an update to an existing one?
          Task dummy;
          if (context.tdb2.get (uuid, dummy))
          {
            if (context.verbose ("sync"))
              out << "  "
                  << colorChanged.colorize (
                       format (STRING_CMD_SYNC_MOD,
                               uuid,
                               from_server.get ("description")))
                  << "\n";
            context.tdb2.modify (from_server, false);
          }
          else
          {
            if (context.verbose ("sync"))
              out << "  "
                  << colorAdded.colorize (
                       format (STRING_CMD_SYNC_ADD,
                               uuid,
                               from_server.get ("description")))
                  << "\n";
            context.tdb2.add (from_server, false);
          }
        }
        else if (line != "")
        {
          sync_key = line;
          context.debug ("Sync key " + sync_key);
        }

        // Otherwise line is blank, so ignore it.
      }

      // Only update everything if there is a new sync_key.  No sync_key means
      // something horrible happened on the other end of the wire.
      if (sync_key != "")
      {
        // Truncate backlog.data, save new sync_key.
        context.tdb2.backlog._file.truncate ();
        context.tdb2.backlog.clear_tasks ();
        context.tdb2.backlog.clear_lines ();
        context.tdb2.backlog.add_line (sync_key + "\n");

        // Present a clear status message.
        if (upload_count == 0 && download_count == 0)
          // Note: should not happen - expect code 201 instead.
          context.footnote (STRING_CMD_SYNC_SUCCESS0);
        else if (upload_count == 0 && download_count > 0)
          context.footnote (format (STRING_CMD_SYNC_SUCCESS2, download_count));
        else if (upload_count > 0 && download_count == 0)
          context.footnote (format (STRING_CMD_SYNC_SUCCESS1, upload_count));
        else if (upload_count > 0 && download_count > 0)
          context.footnote (format (STRING_CMD_SYNC_SUCCESS3, upload_count, download_count));
      }

      status = 0;
    }
    else if (code == "201")
    {
      context.footnote (STRING_CMD_SYNC_SUCCESS_NOP);
      status = 0;
    }
    else if (code == "301")
    {
      std::string new_server = response.get ("info");
      context.config.set ("taskd.server", new_server);
      context.error (STRING_CMD_SYNC_RELOCATE0);
      context.error ("  " + format (STRING_CMD_SYNC_RELOCATE1, new_server));
      status = 2;
    }
    else if (code == "430")
    {
      context.error (STRING_CMD_SYNC_FAIL_ACCOUNT);
      status = 2;
    }
    else
    {
      context.error (format (STRING_CMD_SYNC_FAIL_ERROR,
                             code,
                             response.get ("status")));
      status = 2;
    }

    // Display all errors returned.  This is recommended by the server protocol.
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
  //   - No signal/cable
  else
  {
    context.error (STRING_CMD_SYNC_FAIL_CONNECT);
    status = 1;
  }

  if (context.verbose ("sync"))
    out << "\n";
  output = out.str ();

  // Restore signal handling.
  signal (SIGHUP,    SIG_DFL);
  signal (SIGINT,    SIG_DFL);
  signal (SIGPIPE,   SIG_DFL);
  signal (SIGTERM,   SIG_DFL);
  signal (SIGUSR1,   SIG_DFL);
  signal (SIGUSR2,   SIG_DFL);

#else
  // Without GnuTLS found at compile time, there is no working sync command.
  throw std::string (STRING_CMD_SYNC_NO_TLS);
#endif
  return status;
}

#ifdef HAVE_LIBGNUTLS
////////////////////////////////////////////////////////////////////////////////
bool CmdSync::send (
  const std::string& to,
  const std::string& ca,
  const std::string& certificate,
  const std::string& key,
  const enum TLSClient::trust_level trust,
  const Msg& request,
  Msg& response)
{
  // It is important that the ':' be the *last* colon, in order to support
  // IPv6 addresses.
  auto colon = to.rfind (':');
  if (colon == std::string::npos)
    throw format (STRING_CMD_SYNC_BAD_SERVER, to);

  std::string server = to.substr (0, colon);
  std::string port = to.substr (colon + 1);

  try
  {
    TLSClient client;
    client.debug (context.config.getInteger ("debug.tls"));

    client.trust (trust);
    client.ciphers (context.config.get ("taskd.ciphers"));
    client.init (ca, certificate, key);
    client.connect (server, port);
    client.send (request.serialize () + "\n");

    std::string incoming;
    client.recv (incoming);
    client.bye ();

    response.parse (incoming);
    return true;
  }

  catch (std::string& error)
  {
    context.error (error);
  }

  // Indicate message failed.
  return false;
}
#endif

////////////////////////////////////////////////////////////////////////////////
