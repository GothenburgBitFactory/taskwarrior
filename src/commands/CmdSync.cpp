////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2018, Paul Beckingham, Federico Hernandez.
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
// https://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <CmdSync.h>
#include <sstream>
#include <inttypes.h>
#include <signal.h>
#include <Context.h>
#include <Filter.h>
#include <Color.h>
#include <shared.h>
#include <format.h>
#include <util.h>

////////////////////////////////////////////////////////////////////////////////
CmdSync::CmdSync ()
{
  _keyword               = "synchronize";
  _usage                 = "task          synchronize [initialize]";
  _description           = "Synchronizes data with the Taskserver";
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
    throw std::string ("Command line filters are not supported by this command.");

  // Loog for the 'init' keyword to indicate one-time pending.data upload.
  bool first_time_init = false;
  std::vector <std::string> words = Context::getContext ().cli2.getWords ();
  for (auto& word : words)
  {
    if (closeEnough ("initialize", word, 4))
    {
      if (!Context::getContext ().config.getBoolean ("confirmation") ||
          confirm ("Please confirm that you wish to upload all your tasks to the Taskserver"))
        first_time_init = true;
      else
        throw std::string ("Taskwarrior will not proceed with first-time sync initialization.");
    }
  }

  // If no server is set up, quit.
  std::string connection = Context::getContext ().config.get ("taskd.server");
  if (connection == "" ||
      connection.rfind (':') == std::string::npos)
    throw std::string ("Taskserver is not configured.");

  // Obtain credentials.
  std::string credentials_string = Context::getContext ().config.get ("taskd.credentials");
  if (credentials_string == "")
    throw std::string ("Taskserver credentials malformed.");

  auto credentials = split (credentials_string, '/');
  if (credentials.size () != 3)
    throw std::string ("Taskserver credentials malformed.");

  // This was a Boolean value in 2.3.0, and is a tri-state since 2.4.0.
  std::string trust_value = Context::getContext ().config.get ("taskd.trust");
  if (trust_value != "strict" &&
      trust_value != "ignore hostname" &&
      trust_value != "allow all")
    throw std::string ("The 'taskd.trust' settings may now only contain a value of 'strict', 'ignore hostname' or 'allow all'.");

  enum TLSClient::trust_level trust = TLSClient::strict;
  if (trust_value  == "allow all")
    trust = TLSClient::allow_all;
  else if (trust_value == "ignore hostname")
    trust = TLSClient::ignore_hostname;

  // CA must exist, if provided.
  File ca (Context::getContext ().config.get ("taskd.ca"));
  if (ca._data != "" && ! ca.exists ())
    throw std::string ("CA certificate not found.");

  if (trust == TLSClient::allow_all && ca._data != "")
    throw std::string ("You should either provide a CA certificate or override verification, but not both.");

  File certificate (Context::getContext ().config.get ("taskd.certificate"));
  File key (Context::getContext ().config.get ("taskd.key"));

  if (key.exists () && !certificate.exists ())
    throw std::string ("Taskserver certificate missing.");


  if (certificate.exists () && !key.exists ())
    throw std::string ("Taskserver key missing.");

  // If this is a first-time initialization, send pending.data and
  // completed.data, but not backlog.data.
  std::string payload = "";
  int upload_count = 0;
  if (first_time_init)
  {
    // Delete backlog.data.  Because if we're uploading everything, the list of
    // deltas is meaningless.
    Context::getContext ().tdb2.backlog._file.truncate ();

    auto all_tasks = Context::getContext ().tdb2.all_tasks ();
    for (auto& i : all_tasks)
    {
      payload += i.composeJSON () + '\n';
      ++upload_count;
    }
  }
  else
  {
    std::vector <std::string> lines = Context::getContext ().tdb2.backlog.get_lines ();
    for (auto& i : lines)
    {
      if (i[0] == '{')
        ++upload_count;

      payload += i + '\n';
    }
  }

  // Send 'sync' + payload.
  Msg request;
  request.set ("client",   PACKAGE_STRING);
  request.set ("protocol", "v1");
  request.set ("type",     "sync");
  request.set ("org",      credentials[0]);
  request.set ("user",     credentials[1]);
  request.set ("key",      credentials[2]);

  // Tell the server that this is a full upload, allowing it to perhaps compact
  // its data.
  if (first_time_init)
    request.set ("subtype", "init");

  request.setPayload (payload);

  if (Context::getContext ().verbose ("sync"))
    out << format ("Syncing with {1}", connection)
        << '\n';

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
      Color colorAdded;
      Color colorChanged;
      if (Context::getContext ().color ())
      {
        colorAdded   = Color (Context::getContext ().config.get ("color.sync.added"));
        colorChanged = Color (Context::getContext ().config.get ("color.sync.changed"));
      }

      int download_count = 0;
      payload = response.getPayload ();
      auto lines = split (payload, '\n');

      // Load all tasks, but only if necessary.  There is always a sync key in
      // the payload, so if there are two or more lines, then we have merging
      // to perform, otherwise it's just a backlog.data update.
      if (lines.size () > 1)
        Context::getContext ().tdb2.all_tasks ();

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
          if (Context::getContext ().tdb2.get (uuid, dummy))
          {
            if (Context::getContext ().verbose ("sync"))
              out << "  "
                  << colorChanged.colorize (
                       format ("modify {1} '{2}'",
                               uuid,
                               from_server.get ("description")))
                  << '\n';
            Context::getContext ().tdb2.modify (from_server, false);
          }
          else
          {
            if (Context::getContext ().verbose ("sync"))
              out << "  "
                  << colorAdded.colorize (
                       format ("   add {1} '{2}'",
                               uuid,
                               from_server.get ("description")))
                  << '\n';
            Context::getContext ().tdb2.add (from_server, false);
          }
        }
        else if (line != "")
        {
          sync_key = line;
          Context::getContext ().debug ("Sync key " + sync_key);
        }

        // Otherwise line is blank, so ignore it.
      }

      // Only update everything if there is a new sync_key.  No sync_key means
      // something horrible happened on the other end of the wire.
      if (sync_key != "")
      {
        // Truncate backlog.data, save new sync_key.
        Context::getContext ().tdb2.backlog._file.truncate ();
        Context::getContext ().tdb2.backlog.clear_tasks ();
        Context::getContext ().tdb2.backlog.clear_lines ();
        Context::getContext ().tdb2.backlog.add_line (sync_key + '\n');

        // Present a clear status message.
        if (upload_count == 0 && download_count == 0)
          // Note: should not happen - expect code 201 instead.
          Context::getContext ().footnote ("Sync successful.");
        else if (upload_count == 0 && download_count > 0)
          Context::getContext ().footnote (format ("Sync successful.  {1} changes downloaded.", download_count));
        else if (upload_count > 0 && download_count == 0)
          Context::getContext ().footnote (format ("Sync successful.  {1} changes uploaded.", upload_count));
        else if (upload_count > 0 && download_count > 0)
          Context::getContext ().footnote (format ("Sync successful.  {1} changes uploaded, {2} changes downloaded.", upload_count, download_count));
      }

      status = 0;
    }
    else if (code == "201")
    {
      Context::getContext ().footnote ("Sync successful.  No changes.");
      status = 0;
    }
    else if (code == "301")
    {
      std::string new_server = response.get ("info");
      Context::getContext ().config.set ("taskd.server", new_server);
      Context::getContext ().error ("The server account has been relocated.  Please update your configuration using:");
      Context::getContext ().error ("  " + format ("task config taskd.server {1}", new_server));
      status = 2;
    }
    else if (code == "430")
    {
      Context::getContext ().error ("Sync failed.  Either your credentials are incorrect, or your account doesn't exist on the Taskserver.");
      status = 2;
    }
    else
    {
      Context::getContext ().error (format ("Sync failed.  The Taskserver returned error: {1} {2}",
                             code,
                             response.get ("status")));
      status = 2;
    }

    // Display all errors returned.  This is recommended by the server protocol.
    std::string to_be_displayed = response.get ("messages");
    if (to_be_displayed != "")
    {
      if (Context::getContext ().verbose ("footnote"))
        Context::getContext ().footnote (to_be_displayed);
      else
        Context::getContext ().debug (to_be_displayed);
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
    Context::getContext ().error ("Sync failed.  Could not connect to the Taskserver.");
    status = 1;
  }

  if (Context::getContext ().verbose ("sync"))
    out << '\n';
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
  throw std::string ("Taskwarrior was built without GnuTLS support.  Sync is not available.");
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
    throw format ("Sync failed.  Malformed configuration setting '{1}'", to);

  std::string server = to.substr (0, colon);
  std::string port = to.substr (colon + 1);

  try
  {
    TLSClient client;
    client.debug (Context::getContext ().config.getInteger ("debug.tls"));

    client.trust (trust);
    client.ciphers (Context::getContext ().config.get ("taskd.ciphers"));
    client.init (ca, certificate, key);
    client.connect (server, port);
    client.send (request.serialize () + '\n');

    std::string incoming;
    client.recv (incoming);
    client.bye ();

    response.parse (incoming);
    return true;
  }

  catch (std::string& error)
  {
    Context::getContext ().error (error);
  }

  // Indicate message failed.
  return false;
}
#endif

////////////////////////////////////////////////////////////////////////////////
