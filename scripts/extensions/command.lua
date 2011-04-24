-- Command Extension.
-- Implementing 'random'.

-- Arguments: None
-- Returns:   An 8-element list of installation details.  Only called once, on
--            installation of the extension.
function install ()
  return 'command',                        -- Type
         'random',                         -- Name
         '1.0',                            -- Version
         'Displays a random pending task', -- Description
         'Paul Beckingham',                -- Author
         'paul@beckingham.net',            -- Contact
         'GPLv2',                          -- License
         '© 2011, Göteborg Bit Factory'    -- Copyright
end

-- Arguments: None
-- Returns:   Valid Taskwarrior BNF, minimally defining a production rule that
--            has the same name as the command itself
function syntax ()
  return 'random ::= "random" ;'
end

-- Arguments: None
-- Returns:   1 --> command does not modify data
--            0 --> command modifies data
function read_only ()
  return true
end

-- Arguments: None
-- Returns:   1 --> command displays task ID
--            0 --> no ID displayed
function display_id ()
  return true
end

-- Arguments: None
-- Returns:   1 --> command failed
--            0 --> success
function execute (command_line)
  task_footnote_message ('Not implemented')
  return 1
end

