//! The invocation module handles invoking the commands parsed by the argparse module.

use crate::argparse::{Command, Subcommand};
use crate::settings::Settings;
use taskchampion::{Replica, Server, ServerConfig, StorageConfig, Task, Uuid};
use termcolor::{ColorChoice, StandardStream};

mod cmd;
mod filter;
mod modify;
mod report;

#[cfg(test)]
mod test;

use filter::filtered_tasks;
use modify::apply_modification;
use report::display_report;

/// Invoke the given Command in the context of the given settings
#[allow(clippy::needless_return)]
pub(crate) fn invoke(command: Command, settings: Settings) -> Result<(), crate::Error> {
    log::debug!("command: {:?}", command);
    log::debug!("settings: {:?}", settings);

    let mut w = get_writer();

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
            subcommand: Subcommand::Config { config_operation },
            ..
        } => return cmd::config::execute(&mut w, config_operation, &settings),
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
            subcommand:
                Subcommand::Report {
                    report_name,
                    filter,
                },
            ..
        } => return cmd::report::execute(&mut w, &mut replica, &settings, report_name, filter),

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
            subcommand: Subcommand::Config { .. },
            ..
        } => unreachable!(),
        Command {
            subcommand: Subcommand::Version,
            ..
        } => unreachable!(),
    };
}

// utilities for invoke

/// Get the replica for this invocation
fn get_replica(settings: &Settings) -> anyhow::Result<Replica> {
    let taskdb_dir = settings.data_dir.clone();
    log::debug!("Replica data_dir: {:?}", taskdb_dir);
    let storage_config = StorageConfig::OnDisk { taskdb_dir };
    Ok(Replica::new(storage_config.into_storage()?))
}

/// Get the server for this invocation
fn get_server(settings: &Settings) -> anyhow::Result<Box<dyn Server>> {
    // if server_client_key and server_origin are both set, use
    // the remote server
    let config = if let (Some(client_key), Some(origin), Some(encryption_secret)) = (
        settings.server_client_key.as_ref(),
        settings.server_origin.as_ref(),
        settings.encryption_secret.as_ref(),
    ) {
        let client_key = Uuid::parse_str(&client_key)?;

        log::debug!("Using sync-server with origin {}", origin);
        log::debug!("Sync client ID: {}", client_key);
        ServerConfig::Remote {
            origin: origin.clone(),
            client_key,
            encryption_secret: encryption_secret.as_bytes().to_vec(),
        }
    } else {
        let server_dir = settings.server_dir.clone();
        log::debug!("Using local sync-server at `{:?}`", server_dir);
        ServerConfig::Local { server_dir }
    };
    config.into_server()
}

/// Get a WriteColor implementation based on whether the output is a tty.
fn get_writer() -> StandardStream {
    StandardStream::stdout(if atty::is(atty::Stream::Stdout) {
        ColorChoice::Auto
    } else {
        ColorChoice::Never
    })
}

/// Summarize a task in a single line
fn summarize_task(replica: &mut Replica, task: &Task) -> anyhow::Result<String> {
    let ws = replica.working_set()?;
    let uuid = task.get_uuid();
    if let Some(id) = ws.by_uuid(uuid) {
        Ok(format!("{} - {}", id, task.get_description()))
    } else {
        Ok(format!("{} - {}", uuid, task.get_description()))
    }
}
