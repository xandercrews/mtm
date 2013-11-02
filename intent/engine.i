engine: class

    properties:
        - publish port: +range[1025..65535] = 47000
        - listen port: +range[1025..65535] = 47001
        - publish queue: mq.connection
        - listen queue: mq.connection
        - heartbeats: map of +name to date
        
    make: ctor +copyargs
        doesn't take: heartbeats
        
    update last heartbeat: func
        takes:
            - host: +name
        code:
            lock heartbeats:
                heartbeats[host] = date.now()
            
    start publishing messages: func
        code:
            publish queue.connect(publish port, PUBLISH_MODE)
            
    start listening: func
        code:
            listen queue.connect(listen port, LISTEN_MODE, "domain.*", receive message)
            
    receive message: func
        takes:
            - msg: mq.message
        returns:
            - handled: bool
        code:
            update last heartbeat(msg.from)
            
            handled = true
            # If all the functions below had the same signature, and if all of
            ... them had a reliable mapping between constant and function name,
            ... then we could do the following instead of using "handle":
            ...
            ... function namer: lambda code: `name of(code)`.lower().replace('_', '')
            ... dispatch(namer, arg1, arg2)
            handle msg.code:
                when it == TERMINATE:
                    terminate()
                else:
                    handled = false