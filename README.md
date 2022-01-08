# DBHistory

Directory based history is a tool to maintain a database of command line events in a filterable manner.

## Install

On Ubuntu or any ubuntu based distros it can be installed by using ppa https://launchpad.net/~vargab95/+archive/ubuntu/dbhistory

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
  export PROMPT_COMMAND='RETRN_VAL=$?;dbhistory -a "$(history 1 | sed "s/^[ ]*[0-9]\+[ ]*//" )"'
```

### Configuration file

The configuration file shall be in INI format.
The path of the configuration file can be specified (see in the Command line arguments section).
By default, the configuration file path is \$HOME/.dbhistory.ini

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

#### log_level

Specifies the log level to be used.

The following values can be used:

- 0 = TRACE
- 1 = DEBUG
- 2 = INFO
- 3 = WARNING
- 4 = ERROR

The default value is INFO (2).

### Command line arguments

- -h Shows this help message
- -c Specify configuration file
- -p Cleans up the database
- -a Adds the COMMAND to the history db
- -s Search by applying given regex to pathes
