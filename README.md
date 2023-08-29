# DBHistory

Directory based history is a tool to maintain a database of command line events
in a filterable manner.

## Use case example

Storing bash history per directory can be useful if a developer works on
several projects in parallel. In this case, build, test, lint commands can
vary. When only bash history is used, the developer needs to grep for the used
commands or look up the script or config files, like package.json. By using
dbhistory, this process can be simplified by simply running dbhistory in the
workspace of the project and it'll dump the commands executed on that project.

## Install

On Ubuntu or any ubuntu based distros it can be installed by using ppa
https://launchpad.net/~vargab95/+archive/ubuntu/dbhistory

## Build and install

```
mkdir build
cd build
cmake ..
make
sudo make install
```

## Configure

### Setup shell

#### Bash

Add to the end of .bashrc.

```bash
export PROMPT_COMMAND='RETRN_VAL=$?;dbhistory -a "$(history | tail -n1 | sed "s/^[ ]*[0-9]*[ ]*//" )"'
```

### Configuration file

The configuration file shall be in INI format.  The path of the configuration
file can be specified (see in the Command line arguments section). By default,
the configuration file path is \$HOME/.dbhistory.ini

#### database_path

Specifies the path where the SQLite database will be stored.

Default value is \$HOME/.dbhistory.db

#### log_file_path

Specifies the path where the logs will be stored.

Default value is \$HOME/.dbhistory.log

#### deletion_time_threshold

Time threshold for deleting old history records.
The unit is second.

The default value is -1 which means, there is no time threshold for all entries.

#### max_command_length

Specifies the maximum length of a command which can be stored.

Default is 4096.

#### use_pinnings

Specifies whether the pinnings feature is enabled.

Default is 1.

#### log_level

Specifies the log level to be used.

The following values can be used:

- 0 = TRACE
- 1 = DEBUG
- 2 = INFO
- 3 = WARNING
- 4 = ERROR

The default value is INFO (2).

#### result_limit

Specifies the default limit of result records. Only the last n records will be
printed by dbhistory to avoid slow listing due to printing thousands of
commands. Can be overriden by the -l command line argument.

The default value is 100.

### Command line arguments

- -h Shows this help message
- -f Specify configuration file
- -c Cleans up the database
- -a Adds the COMMAND to the history db
- -s Search by applying given regex to pathes
- -p Pin a command to the current folder
- -u Unpin a command by ID
- -o Show only the history without pinnings
- -l Limit the number of results

## Statistics

As dbhistory uses SQLite3 by default, it does not make sense to write
predefined statistic commands into the tool. If you're interested in history
statistics, it's recommended to open the database file with an sqlite browser
and create views for the statistics. For example, a history count per path
statistics view can be created by executing the following SQL statement.

```sql
CREATE VIEW history_count_per_path 
AS
SELECT path, COUNT(history.path_id) AS occurence
FROM history JOIN path_map ON path_map.id == history.path_id
GROUP BY history.path_id 
ORDER BY occurence DESC 
```
