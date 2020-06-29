# DBHistory

Directory based history

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

Add to the end of .bashrc

```
  trap 'dbhistory -a "$BASH_COMMAND"' DEBUG
```

### Configuration file

#### database_path

#### log_file_path

#### deletion_time_threshold

#### max_command_length

#### log_level

### Command line arguments

-h Shows this help message
-c Specify configuration file
-p Cleans up the database
-a Adds the COMMAND to the history db
-s Search by applying given regex to pathes\
