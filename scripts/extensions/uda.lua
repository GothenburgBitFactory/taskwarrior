-- User Defined Attribute Extension.
-- Implementing 'priority'.

-- Arguments: None
-- Returns:   An 8-element list of installation details.  Only called once, on
--            installation of the extension.
function install ()
  return 'uda',                            -- Type
         'priority',                       -- Name
         1.0,                              -- Version
         'Implements priority attribute',  -- Description
         'Paul Beckingham',                -- Author
         'paul@beckingham.net',            -- Contact
         'GPLv2',                          -- License
         '© 2011, Göteborg Bit Factory'    -- Copyright
end


-- Arguments: None
-- Returns:   Data type
function type ()
  return 'custom'
end

-- Arguments: proposed value
-- Returns:   1 --> allowed
--            0 --> disallowed
function allowed (value)
  if value == 'H' ||
     value == 'M' ||
     value == 'L' ||
     value == '' then
    return 1
  end

  return 0
end

-- Arguments: left and right values to compare
-- Returns:   1 --> left < right
--            0 --> left >= right
function compare (left, right)
  if left == 'M' && right == 'H' then
    return 1
  elseif left == 'L' && (right == 'H' || right == 'M') then
    return 1
  elseif left == '' then
    return 1
  end

  return 0
end

-- Arguments: Raw data
-- Returns:   Formatted data
-- Note:      Shown here is a pass-through format, doing no formatting.  This is
--            the default behavior if the format function is not implemented.
function format (value)
  return value
end

