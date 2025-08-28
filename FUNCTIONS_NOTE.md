# Script Function Note

## `gml_Script_point@TrellisPoints@TrellisPoint`

Return the TrellisPoint with the given name. Most likely from `__tp (TrellisPoints)`.

### self

Seems to be `__tp` from global instance.

### other

An instance (not sure where to get it yet). Passing in null should be fine.

### return

(struct) `TrellisPoint` with members like

- `base_name`
- `name`
- `location_position` (LocationPosition) that has `location_id` and `pos` (`x`, `y`)

### args[0] of 1

(string) The name of the point to get. E.g. `bathhouse/Dozy`.

## `gml_Script_schedule_current_destination@T2r@T2r`

Return the name of the current destination for the given NPC's ID?

### self

Seems to be `__t2r` from global instance.

### other

An instance (not sure where to get it yet). Passing in null should be fine.

### return

(string) The name of the `TrellisPoint` e.g. `town/manor_routine_entrance`.

### args[0] of 1

(number) an NPC's ID? E.g. `3.0`
