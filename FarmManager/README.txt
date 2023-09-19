Python scripts in this directory implement a rudimentary way to launch SorterHunter and harvest results spanning multiple threads and machines.
It performs remote execution using ssh (assuming no password required by adding our public key to authorized_keys on remote machine).
Local machine should have 'unbuffer' installed ('expect' package) to fix an I/O buffering issue.

Edit the "joblist" list in TestFarm.py as required, one job per thread, local or remote.

The TestFarm.py script parses all reported improvements from the SorterHunter instances and just shows the overall best ones.
JSON parser is used because the syntax nearly matches the SorterHunter output.
The ThreadRunner.py module is just a wrapper objectifying processes



Note on SSHD configuration:

Target machine's /etc/ssh/sshd_config needs to allow sufficient sessions.

=> Adapt to allow at least the number of threads we are launching. Extract from the manual:

     MaxSessions
             Specifies the maximum number of open shell, login or
             subsystem (e.g. sftp) sessions permitted per network
             connection.  Multiple sessions may be established by
             clients that support connection multiplexing.  Setting
             MaxSessions to 1 will effectively disable session
             multiplexing, whereas setting it to 0 will prevent all
             shell, login and subsystem sessions while still permitting
             forwarding.  The default is 10.                                   => So let's make this e.g. 20 if we need 16 threads

     MaxStartups
             Specifies the maximum number of concurrent unauthenticated
             connections to the SSH daemon.  Additional connections will
             be dropped until authentication succeeds or the
             LoginGraceTime expires for a connection.  The default is
             10:30:100.                                                        => So let's make this e.g. 20:30:100 if we need 16 threads

             Alternatively, random early drop can be enabled by
             specifying the three colon separated values start:rate:full
             (e.g. "10:30:60").  sshd(8) will refuse connection attempts
             with a probability of rate/100 (30%) if there are currently
             start (10) unauthenticated connections.  The probability
             increases linearly and all connection attempts are refused
             if the number of unauthenticated connections reaches full
             (60).
             
=> To apply on target:             
    sudo service sshd restart
