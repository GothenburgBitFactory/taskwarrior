-- User Defined Attribute Extension.
-- Implementing 'priority'.

-- Arguments: None
-- Returns:   An 8-element list of installation details.  Only called once, on
--            installation of the extension.
function install ()
  return 'uda',                            -- Type
         'priority',                       -- Name
         '1.0',                            -- Version
         'Implements priority attribute',  -- Description
         'Paul Beckingham',                -- Author
         'paul@beckingham.net',            -- Contact
         'MIT',                            -- License
         '© 2013, Göteborg Bit Factory'    -- Copyright
end

-- Arguments: None
-- Returns:   Data type
function type ()
  return 'custom'
end

-- Arguments: None
-- Returns:   List of allowable values
function allowed ()
  return 'H', 'M', 'L', ''
end

-- Arguments: Left and right values to compare
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
--            also the default behavior if the format function is not
--            implemented.
function format (value)
  return value
end

-- Arguments: Value
-- Returns:   Urgency Term
-- Note:      Should reference rc.urgency.<field>.coefficient
function urgency (uuid)
  coefficient = task_get ('rc.urgency.priority.coefficient')

  -- TODO Urgency calculation here

  return coefficient * 1.0
end

