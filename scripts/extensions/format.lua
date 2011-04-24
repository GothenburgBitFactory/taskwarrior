-- Format Extension.
-- Implementing 'uuid.short'

-- Arguments: None
-- Returns:   An 8-element list of installation details.  Only called once, on
--            installation of the extension.
function install ()
  return 'format',                         -- Type
         'uuid.short',                     -- Name
         '1.0',                            -- Version
         'Provides short formatted UUIDs', -- Description
         'Paul Beckingham',                -- Author
         'paul@beckingham.net',            -- Contact
         'GPLv2',                          -- License
         '© 2011, Göteborg Bit Factory'    -- Copyright
end

-- Argument: Value to be formatted
-- Returns:  Formatted value
function format (value)
  return string.sub (value, 0, 8)
end

