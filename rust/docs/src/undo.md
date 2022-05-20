# Undo

It's easy to make a mistake: mark the wrong task as done, or hit enter before noticing a typo in a tag name.
The `ta undo` command makes it just as easy to fix the mistake, by effectively reversing the most recent change.
Multiple invocations of `ta undo` can be used to undo multiple changes.

The limit of this functionality is that changes which have been synchronized to the server (via `ta sync`) cannot be undone.
