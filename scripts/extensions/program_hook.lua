-- Program Hook Extension.
-- Implementing goodbye message.

-- Arguments: None
-- Returns:   An 8-element list of installation details.  Only called once, on
--            installation of the extension.
function install ()
  return 'program',                        -- Type
         'goodbye',                        -- Name
         '1.0',                            -- Version
         'Simply says goodbye',            -- Description
         'Paul Beckingham',                -- Author
         'paul@beckingham.net',            -- Contact
         'MIT',                            -- License
         '© 2011, Göteborg Bit Factory'    -- Copyright
end

-- Arguments: None
-- Returns:   String identifying valid program hook
function hook ()
  return 'on-exit'
end

-- Arguments: None
-- Returns:   0 --> success only
function goodbye ()
  print ('Goodbye.')
end

