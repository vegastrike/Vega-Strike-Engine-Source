# diskobject

The diskobject library provides an interface for reading and storing information
to and from disk, primarily for the saved game format and similar needs.

## Background

The original Vega Strike Engine up through 0.9.x would store the saved game data in
several places:

1. `~/.vegastrike/save`
2. `~/.vegastrike/serialized_xml`

A file under the first would have a matching directory under the second containing
more data.

## Goal

Instead of spreading the data across multiple places, provide it all into a single
compressed file that contains the same information as previously. This will help make
the data more portable, easier to find, and easier to cleanup.

In general, the Vega Strike Engine simply reads and writes the files as a whole at one
time. Instead of updating a file it just writes a new one or overwrites a file.
This simplifies the process since `w+b` can be used when writing which will truncate
existing files.

## Examples

### Listing Entries
```
std::string saved_file = "~/.vegastrike/saved/auto_save";
auto disk_object = vegastrike::diskobject::DiskObject(saved_file);

std::deque<std::string> entries;
int disk_object_error = disk_object.ListEntries(entries);
```

### Reading

```
std::string saved_file = "~/.vegastrike/saved/auto_save";
auto disk_object = vegastrike::diskobject::DiskObject(saved_file);

std::string primary_data;
int disk_object_error = disk_object.ReadEntry(primary, primary_data);
```
