use crate::settings;
use clap::Arg;
use config::{Config, ConfigError};
use failure::{format_err, Fallible};
use std::cell::{Ref, RefCell};
use taskchampion::{server, Replica, ReplicaConfig, ServerConfig, Task, Uuid};

pub(super) fn task_arg<'a>() -> Arg<'a, 'a> {
    Arg::with_name("task")
        .help("task id or uuid")
        .required(true)
}

pub(super) fn get_task<S: AsRef<str>>(replica: &mut Replica, task_arg: S) -> Fallible<Task> {
    let task_arg = task_arg.as_ref();

    // first try treating task as a working-set reference
    if let Ok(i) = task_arg.parse::<usize>() {
        if let Some(task) = replica.get_working_set_task(i)? {
            return Ok(task);
        }
    }

    if let Ok(uuid) = Uuid::parse_str(task_arg) {
        if let Some(task) = replica.get_task(&uuid)? {
            return Ok(task);
        }
    }

    Err(format_err!("Cannot interpret {:?} as a task", task_arg))
}

/// A command invocation contains all of the necessary regarding a single invocation of the CLI.
#[derive(Debug)]
pub struct CommandInvocation {
    pub(crate) subcommand: Box<dyn super::SubCommandInvocation>,
    settings: RefCell<Config>,
}

impl CommandInvocation {
    pub(crate) fn new(subcommand: Box<dyn super::SubCommandInvocation>) -> Self {
        Self {
            subcommand,
            settings: RefCell::new(Config::default()),
        }
    }

    pub fn run(self) -> Fallible<()> {
        self.subcommand.run(&self)
    }

    // -- utilities for command invocations

    pub(super) fn get_settings(&self) -> Fallible<Ref<Config>> {
        {
            // use the special `_loaded" config value to detect whether we have
            // loaded the configuration yet
            let mut settings = self.settings.borrow_mut();
            if let Err(ConfigError::NotFound(_)) = settings.get_bool("_loaded") {
                settings.merge(settings::read_settings()?)?;
                settings.set("_loaded", true)?;
            }
        }
        Ok(self.settings.borrow())
    }

    pub(super) fn get_replica(&self) -> Fallible<Replica> {
        let settings = self.get_settings()?;
        let taskdb_dir = settings.get_str("data_dir")?.into();
        log::debug!("Replica data_dir: {:?}", taskdb_dir);
        let replica_config = ReplicaConfig { taskdb_dir };
        Ok(Replica::from_config(replica_config)?)
    }

    pub(super) fn get_server(&self) -> Fallible<Box<dyn server::Server>> {
        let settings = self.get_settings()?;
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
}
