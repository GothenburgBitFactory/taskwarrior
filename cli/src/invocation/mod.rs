//! The invocation module handles invoking the commands parsed by the argparse module.

use crate::argparse::{Command, Subcommand};
use config::Config;
use failure::Fallible;
use taskchampion::{server, Replica, ReplicaConfig, ServerConfig, Uuid};
use termcolor::{ColorChoice, StandardStream};

mod cmd;
mod filter;
mod modify;

use filter::filtered_tasks;
use modify::apply_modification;

/// Invoke the given Command in the context of the given settings
pub(crate) fn invoke(command: Command, settings: Config) -> Fallible<()> {
    log::debug!("command: {:?}", command);
    log::debug!("settings: {:?}", settings);

    let mut w = get_writer()?;

    // This function examines the command and breaks out the necessary bits to call one of the
    // `execute` functions in a submodule of `cmd`.

    // match the subcommands that do not require a replica first, before
    // getting the replica
    match command {
        Command {
            subcommand: Subcommand::Help { summary },
            command_name,
        } => return cmd::help::execute(&mut w, command_name, summary),
        Command {
            subcommand: Subcommand::Version,
            ..
        } => return cmd::version::execute(&mut w),
        _ => {}
    };

    let mut replica = get_replica(&settings)?;
    match command {
        Command {
            subcommand: Subcommand::Add { modification },
            ..
        } => return cmd::add::execute(&mut w, &mut replica, modification),

        Command {
            subcommand:
                Subcommand::Modify {
                    filter,
                    modification,
                },
            ..
        } => return cmd::modify::execute(&mut w, &mut replica, filter, modification),

        Command {
            subcommand: Subcommand::List { report },
            ..
        } => return cmd::list::execute(&mut w, &mut replica, report),

        Command {
            subcommand: Subcommand::Info { filter, debug },
            ..
        } => return cmd::info::execute(&mut w, &mut replica, filter, debug),

        Command {
            subcommand: Subcommand::Gc,
            ..
        } => return cmd::gc::execute(&mut w, &mut replica),

        Command {
            subcommand: Subcommand::Sync,
            ..
        } => {
            let mut server = get_server(&settings)?;
            return cmd::sync::execute(&mut w, &mut replica, &mut server);
        }

        // handled in the first match, but here to ensure this match is exhaustive
        Command {
            subcommand: Subcommand::Help { .. },
            ..
        } => unreachable!(),
        Command {
            subcommand: Subcommand::Version,
            ..
        } => unreachable!(),
    }
}

// utilities for invoke

/// Get the replica for this invocation
fn get_replica(settings: &Config) -> Fallible<Replica> {
    let taskdb_dir = settings.get_str("data_dir")?.into();
    log::debug!("Replica data_dir: {:?}", taskdb_dir);
    let replica_config = ReplicaConfig { taskdb_dir };
    Ok(Replica::from_config(replica_config)?)
}

/// Get the server for this invocation
fn get_server(settings: &Config) -> Fallible<Box<dyn server::Server>> {
    let client_id = settings.get_str("server_client_id")?;
    let client_id = Uuid::parse_str(&client_id)?;
    let origin = settings.get_str("server_origin")?;
    log::debug!("Using sync-server with origin {}", origin);
    log::debug!("Sync client ID: {}", client_id);
    Ok(server::from_config(ServerConfig::Remote {
        origin,
        client_id,
    })?)
}

/// Get a WriteColor implementation based on whether the output is a tty.
fn get_writer() -> Fallible<StandardStream> {
    Ok(StandardStream::stdout(if atty::is(atty::Stream::Stdout) {
        ColorChoice::Auto
    } else {
        ColorChoice::Never
    }))
}
