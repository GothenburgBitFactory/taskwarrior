-- Task Hook Extension.
-- Implementing encouragement message.

-- Arguments: None
-- Returns:   An 8-element list of installation details.  Only called once, on
--            installation of the extension.
function install ()
  return 'task',                           -- Type
         'encourage',                      -- Name
         '1.0',                            -- Version
         'Positive feedback',              -- Description
         'Paul Beckingham',                -- Author
         'paul@beckingham.net',            -- Contact
         'GPLv2',                          -- License
         '© 2011, Göteborg Bit Factory'    -- Copyright
end

-- Arguments: None
-- Returns:   String identifying valid program hook
function hook ()
  return 'on-task-complete'
end

-- Arguments: None
-- Returns:   1 --> failure
--            0 --> success
function encourage ()
  -- Only provide encouragement if the verbosity settings allow it.
  verbosity = task_get ('rc.verbose')
  if string.find (verbosity, 'encourage') ~= nil
  then
    task_footnote_message ('Good work.')
  end
  return 0
end

