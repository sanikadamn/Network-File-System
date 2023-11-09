# HDFS Project Document

# Abbreviations
- Client: C
- Name Server: NS
- Storage Server: SS

## Client
The client will have to send a request to the NS in order to connect to the DFS. C will send request packets to the NS, and the NS will return the SS that contains the requested file (if not creating, deleting or copying) or an ACK/feedback on the operation (otherwise).

### Creating files/folders
C sends a packet with the ACTION and the PATH, to NS. The file system then handles the actual execution, with C then receiving feedback on its status from the NS.

### Copying files/folders (copy, not cut)
C will send a packet with the corresponding ACTION and the SOURCE and DESTINATION paths to the NS. After the operation has been completed, it will then receive feedback from the NS.

### Read, write, retrieve information
C sends the ACTION and the PATH to NS. NS returns the appropriate SS's IP address and client port. Subsequently, the client communicates with the SS directly to send or receive information.

## Storage Server
Each server $S_i$ has a collection of files $C_i$, each of which have the following metadata for every file stored:
- File path on the server
- File path on the actual drive
- Last modified time (Time according to NS)
- File permissions (How to do this? Store usernames?)
- File size

### Creating files/folders
The file/folder path provided shall assume that the parent folder is created. 

To create a new file, the following steps are performed:
- The NS sends a CREATE packet to the chosen SS. The packet contains the filepath and permissions for the file to be created
- The SS will mark an empty file in its own collection $C_i$. 
- The NS will receive the updated heartbeat that will acknowledge the creation of a new file in the SS. Once the NS has received the heartbeat, it shall convey it to the client using an ACK packet.

#### NS Responses:
- ``ENOSERV``: No servers left
- ``ENOPERM``: Invalid Permissions
- ``EINVAL``: Invalid path given

#### SS Responses:
- ``EFULL``: SS is full and has no further space

### Reading a file
To read a file, the following steps are performed:
- The client sends a ``READ`` packet specifying the file path to read.
- The NS identifies the SS to be accessed, based on a variety of parameters and sends the details of the SS (IP address, Port No) to the client. If there are no SS available, it shall return a ``ENOSERV`` packet. If the file path is not found or the client does not have sufficient permissions, it shall return a ``ENOFOUND`` packet.
- The client sends a ``READ`` packet to the SS with the file path
- The SS looks up the provided file path in the mappings maintained, and checks if a write lock is being held
- If no write lock is being held, it is free to read and send to the client. Else, it returns ``EBUSY``, and the client has to retry after sometime.

#### NS Errors:
- ``ENOTFOUND``: File path not found
- ``ENOSERV``: No servers are available

#### SS Errors:
- ``EBUSY``: File is being written, client should try after some time

### Writing a file
- The client sends a ``WRITE`` packet specifying the file path to write.
- The NS identifies the SS to be accessed, based on a variety of parameters and sends the details of the SS (IP address, Port No) to the client. If there are no SS available, it shall return a ``ENOSERV`` packet. If the file path is not found or the client does not have sufficient permissions, it shall return a ``ENOFOUND`` packet.
- The client sends a ``WRITE`` packet to the SS with the file path
- The SS checks if a read/write lock is held currently. If yes, it shall send an ``EBUSY`` to the client, else it shall hold the lock
- On completing the write, the SS releases the write lock and sends a new heartbeat, containing the updated the file details.

#### NS Errors:
- ``ENOTFOUND``: File path not found
- ``ENOSERV``: No servers are available

#### SS Errors:
- ``EBUSY``: File is being written, client should try after some time

# Defining Packets
**Note**: Some of these packets may be combined and denoted by one struct at the implementation level. This categorization is largely semantic.

## C-NS Packets
 
### Expect Redirect ONE-PATH Packets (C &rarr; NS)
Contain
- `ACTION` -- what is to be done
    - `READ`
    - `WRITE`
    - `SIZE`
    - `PERMS`
- `PATH` -- the file/directory path

### Expect Feedback ONE-PATH Packets (C &rarr; NS)
Contain
- `ACTION` -- what is to be done
    - `CREATE`
    - `DELETE`
- `PATH` -- the file/directory path

### TWO-PATH Packets (C &rarr; NS)
They are only used by the client to copy files or directories. They contain
- `SRC` -- the location from which to be copied
- `DEST` -- the location where to copy

They also expect feedback

### Feedback Packets (NS &rarr; C)
Used by the NS to send feedback on the task's status to C. Contains only one segment.

### IP-Port/Redirect Packets (NS &rarr; C)
Used by the NS to return an SS's IP and Port to C. Contains
- `IP` of the selected SS
- `Port` to connect to the selected SS.

## C-SS Packets

### STOP Packets (SS &rarr; C)
Used by the SS to indicate end of stream to C (also to NS). [Can be combined with feedback in implementation]

### ONE-PATH Packets (C &rarr; SS)
Contain
- `ACTION` -- what is to be done
    - `READ`
    - `WRITE`
    - `SIZE`
    - `PERMS`
- `PATH` -- the file/directory path