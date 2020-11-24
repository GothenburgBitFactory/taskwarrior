/// Define a Command type implementing SubCommand with the enclosed methods (`decorate_app` and
/// `arg_match`), along with a module-level `cmd` function as the parent module expects.
macro_rules! define_subcommand {
    ($($f:item) +) => {
        struct Command;

        pub(super) fn cmd() -> Box<dyn crate::cmd::SubCommand> {
            Box::new(Command)
        }

        impl crate::cmd::SubCommand for Command {
            $($f)+
        }
    }
}

/// Define an Invocation type implementing SubCommandInvocation with the enclosed methods.
macro_rules! subcommand_invocation {
    ($($f:item) +) => {
        impl crate::cmd::SubCommandInvocation for Invocation {
            $($f)+

            #[cfg(test)]
            fn as_any(&self) -> &dyn std::any::Any {
                self
            }
        }
    }

}

/// Parse the first argument as a command line and convert the result to an Invocation (which must
/// be in scope).  If the conversion works, calls the second argument with it.
#[cfg(test)]
macro_rules! with_subcommand_invocation {
    ($args:expr, $check:expr) => {
        let parsed = crate::parse_command_line($args).unwrap();
        let si = parsed
            .subcommand
            .as_any()
            .downcast_ref::<Invocation>()
            .expect("SubComand is not of the expected type");
        ($check)(si);
    };
}
