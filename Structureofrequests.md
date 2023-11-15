# Structure of the requests that the Client sends to the NS

```c
typedef struct req{
    int reqID;
    char path[1024];
} Requests;
```

The reqID is the type of the request that the client wants to make.

``0`` for Write \\
``1`` for Read \\
``2`` for Delete \\
``3`` for Create \\
``4`` for List \\ 
``5`` for More Info \\


# Structure of responses that the NS sends to the Client

```c
typedef struct res{
    int errortype;
    struct 
} Responses;
```
