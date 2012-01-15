-- DOM Extension.
-- Implementing 'system.load.average'.

-- Arguments: None
-- Returns:   An 8-element list of installation details.  Only called once, on
--            installation of the extension.
function install ()
  return 'dom',                            -- Type
         'system.load.average',            -- Name
         '1.0',                            -- Version
         'Provides access to system load', -- Description
         'Paul Beckingham',                -- Author
         'paul@beckingham.net',            -- Contact
         'MIT',                            -- License
         '© 2012, Göteborg Bit Factory'    -- Copyright
end

-- Arguments: The DOM reference to evaluate
-- Returns:   The value from the DOM lookup
function lookup (name)
  if name == 'system.load.average'
  then
    return 1.23 -- Fake load average
  end
end

