//! Reimplementation of the CLI2 class in TaskWarrior.
//!
//! This class is sparsely tested in TaskWarrior, but the intent is to replicate its functionality
//! reliably enough that any command-line accepted by TaskWarrior will also be accepted by this
//! implementation.

use super::a2::A2;
use crate::util::lexer::{dequote, read_word_quoted, was_quoted, Lexer, Type};
use std::collections::{HashMap, HashSet};

#[derive(Default)]
pub(crate) struct CLI2 {
    entities: HashMap<String, HashSet<String>>,
    aliases: HashMap<String, String>,
    original_args: Vec<A2>,
    args: Vec<A2>,
    id_ranges: Vec<(String, String)>,
    uuid_list: Vec<String>,
    context_filter_added: bool,
}

impl CLI2 {
    pub(crate) fn new() -> CLI2 {
        CLI2 {
            ..Default::default()
        }
    }

    /// Add an alias
    pub(crate) fn alias<S1: Into<String>, S2: Into<String>>(&mut self, name: S1, value: S2) {
        self.aliases.insert(name.into(), value.into());
    }

    /// Add an entity category thing ??
    pub(crate) fn entity<S1: Into<String>, S2: Into<String>>(&mut self, category: S1, name: S2) {
        self.entities
            .entry(category.into())
            .or_insert_with(|| HashSet::new())
            .insert(name.into());
    }

    /// Capture a single argument, tagged as ORIGINAL
    pub(crate) fn add<S: Into<String>>(&mut self, argument: S) {
        let mut arg = A2::new(argument, Type::Word);
        arg.tag("ORIGINAL");
        self.original_args.push(arg);
        self.args.clear();
    }

    /// Capture a set of arguments, inserted immediately after the binary.
    /// There must be at least one argument set already. The new args are not
    /// tagged as ORIGINAL.
    ///
    /// Note that this is in no way equivalent to calling `add` in a loop!
    pub(crate) fn add_args<S: Into<String>>(&mut self, arguments: Vec<S>) {
        let mut replacement = vec![self.original_args[0].clone()];
        for arg in arguments {
            replacement.push(A2::new(arg, Type::Word));
        }
        for arg in self.original_args.drain(1..) {
            replacement.push(arg);
        }
        self.original_args = replacement;
        self.args.clear();
    }

    /// Perform the command-line analysis after arguments are added with `add` and `add_args`.
    pub(crate) fn analyze(&mut self) {
        self.args.clear();
        self.handle_arg0();
        self.lex_arguments();
        // self.alias_expansion(); - TODO
        if !self.find_command() {
            self.default_command();
            assert!(self.find_command()); // default_command guarantees this
        }
        // self.demotion(); - TODO
        // self.canonicalizeNames(); - TODO
        // self.categorizeArgs(); - TODO
        // self.parenthesizeOriginalFilter(); - TODO
    }

    /// Handle the first argument, indicating the invoked binary.
    fn handle_arg0(&mut self) {
        // NOTE: this omits the special handling for "cal" and "calendar"
        self.original_args[0].tag("BINARY");
    }

    /// Use the lexer to process all arguments (except the first, handled by handle_arg0).
    ///
    /// All arguments must be individually and wholly recognized by the Lexer. Any argument not
    /// recognized is considered a lexer::Type::Word.
    ///
    /// As a side effect, tags all arguments after a terminator ('--') with TERMINATED.
    fn lex_arguments(&mut self) {
        let mut terminated = false;

        // Note: Starts iterating at index 1, because ::handleArg0 has already
        //       processed it.
        for arg in &self.original_args[1..] {
            let raw = arg.get_attribute("raw").unwrap();
            let quoted = was_quoted(raw);

            // Process single-token arguments.
            let mut lex = Lexer::new(raw);
            match lex.token() {
                // if we got a token and it goes to EOS (quoted pairs automatically go to EOS)
                Some((lexeme, mut lextype))
                    if lex.is_eos() || (quoted && lextype == Type::Pair) =>
                {
                    if !terminated && lextype == Type::Separator {
                        terminated = true;
                    } else if terminated {
                        lextype = Type::Word;
                    }

                    let mut lexed_arg = A2::new(raw, lextype);
                    if terminated {
                        lexed_arg.tag("TERMINATED");
                    }
                    if quoted {
                        lexed_arg.tag("QUOTED");
                    }
                    if arg.has_tag("ORIGINAL") {
                        lexed_arg.tag("ORIGINAL");
                    }
                    self.args.push(lexed_arg)
                }
                // ..otherwise, process "muktiple-token" arguments
                _ => {
                    // TODO: this is kind of insane and almost certainly wrong, but
                    // implements what the C++ code does..
                    let quote = "'";
                    let escaped = format!("'{}'", raw.replace(quote, "\\'"));
                    let mut lexed_arg;
                    if let Some((word, _)) = read_word_quoted(&escaped, quote, 0) {
                        let word = dequote(&word, "'\"");
                        lexed_arg = A2::new(word, Type::Word);
                    } else {
                        // "This branch may have no use-case"!
                        lexed_arg = A2::new(raw, Type::Word);
                        lexed_arg.tag("UNKNOWN");
                    }
                    if quoted {
                        lexed_arg.tag("QUOTED");
                    }
                    if arg.has_tag("ORIGINAL") {
                        lexed_arg.tag("ORIGINAL");
                    }
                    self.args.push(lexed_arg)
                }
            }
        }
        /*
        println!("lexed args:");
        for arg in &self.args {
            println!("{:?}", arg);
        }
        */
    }

    /// Scan all arguments and if any are an exact match for a command name, then tag as CMD. If an
    /// argument is an exact match for an attribute, despite being an inexact match for a command,
    /// then it is not a command.
    fn find_command(&mut self) -> bool {
        for (i, arg) in self.args.iter().enumerate() {
            let raw = arg.get_attribute("raw").unwrap();
            let canonical;

            if self.exact_match_entity("cmd", raw) {
                canonical = raw.into();
            } else if self.exact_match_entity("attribute", raw) {
                continue;
            } else if let Some(cannon) = self.canonicalize_entity("cmd", raw) {
                canonical = cannon;
            } else {
                continue;
            }

            let mut arg = arg.clone();
            arg.set_attribute("canonical", canonical);
            arg.tag("CMD");

            // TODO: apply "command DNA"

            self.args[i] = arg;

            return true;
        }

        false
    }

    /// Set a default command argument.  Look for situations that require defaults:
    ///
    /// 1. If no command was found, and no ID/UUID, and if rc.default.command is
    ///    configured, inject the lexed tokens from rc.default.command.
    ///
    /// 2. If no command was found, but an ID/UUID was found, then assume a command
    ///    of 'information'.
    fn default_command(&mut self) {
        let mut found_command = false;
        let mut found_sequence = false;

        for arg in &self.args {
            if arg.has_tag("CMD") {
                found_command = true;
            }
            if arg.lextype == Type::Uuid || arg.lextype == Type::Number {
                found_sequence = true;
            }
        }

        if !found_command {
            if !found_sequence {
                unreachable!(); // TODO (requires default.command, context, etc.)
            } else {
                let mut info = A2::new("information", Type::Word);
                info.tag("ASSUMED");
                info.tag("CMD");
                self.args.insert(0, info);
            }
        }
    }

    /// Search for 'value' in _entities category, return canonicalized value.
    fn canonicalize_entity(&self, category: &str, value: &str) -> Option<String> {
        // TODO: for the moment this only accepts exact matches
        if let Some(names) = self.entities.get(category) {
            if names.contains(value) {
                Some(value.into())
            } else {
                None
            }
        } else {
            None
        }
    }

    /// Search for exact 'value' in _entities category.
    fn exact_match_entity(&self, category: &str, value: &str) -> bool {
        if let Some(names) = self.entities.get(category) {
            names.contains(value)
        } else {
            false
        }
    }
}

#[cfg(test)]
mod test {
    use super::*;

    fn assert_args(args: &Vec<A2>, exp: Vec<&str>) {
        assert_eq!(
            args.iter().map(|a| format!("{:?}", a)).collect::<Vec<_>>(),
            exp.iter().map(|s| s.to_string()).collect::<Vec<_>>(),
        );
    }

    #[test]
    fn alias() {
        let mut c = CLI2::new();
        c.alias("foo", "bar");
        assert_eq!(c.aliases.get("foo"), Some(&"bar".to_string()));
    }

    #[test]
    fn entities() {
        let mut c = CLI2::new();
        c.entity("cat", "foo");
        c.entity("cat", "bar");
        let mut exp = HashSet::new();
        exp.insert("foo".to_string());
        exp.insert("bar".to_string());
        assert_eq!(c.entities.get("cat"), Some(&exp));
    }

    #[test]
    fn add() {
        let mut c = CLI2::new();
        c.add("foo");
        c.add("bar");
        assert_eq!(
            c.original_args
                .iter()
                .map(|a| format!("{:?}", a))
                .collect::<Vec<_>>(),
            vec![
                "A2{Word, ORIGINAL, raw=\"foo\"}",
                "A2{Word, ORIGINAL, raw=\"bar\"}"
            ]
        );
    }

    #[test]
    fn add_args() {
        let mut c = CLI2::new();
        c.add("0");
        c.add("1");
        c.add("2");
        c.add_args(vec!["foo", "bar"]);
        assert_args(
            &c.original_args,
            vec![
                "A2{Word, ORIGINAL, raw=\"0\"}",
                "A2{Word, raw=\"foo\"}",
                "A2{Word, raw=\"bar\"}",
                "A2{Word, ORIGINAL, raw=\"1\"}",
                "A2{Word, ORIGINAL, raw=\"2\"}",
            ],
        );
    }

    #[test]
    fn analyze_example_cmdline() {
        let mut c = CLI2::new();
        c.entity("cmd", "next");
        c.add("arg0");
        c.add("rc.gc=0");
        c.add("next");
        c.add("+PENDING");
        c.add("due:tomorrow");
        c.analyze();
        assert_args(
            &c.args,
            vec![
                "A2{Pair, CONFIG, ORIGINAL, modifier=\"gc\", name=\"rc\", raw=\"rc.gc=0\", separator=\"=\", value=\"0\"}",
                "A2{Identifier, CMD, ORIGINAL, canonical=\"next\", raw=\"next\"}",
                "A2{Tag, ORIGINAL, name=\"PENDING\", raw=\"+PENDING\", sign=\"+\"}",
                "A2{Pair, ORIGINAL, modifier=\"\", name=\"due\", raw=\"due:tomorrow\", separator=\":\", value=\"tomorrow\"}",
            ],
        );
    }

    #[test]
    fn exact_match_entity() {
        let mut c = CLI2::new();
        c.entity("cmd", "next");
        c.entity("cmd", "list");
        assert!(c.exact_match_entity("cmd", "next"));
        assert!(!c.exact_match_entity("cmd", "bar"));
        assert!(!c.exact_match_entity("foo", "bar"));
    }
}
