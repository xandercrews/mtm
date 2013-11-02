server engine: class

    properties:
        - helpers: list of +name
        - publish port: +range[1025..65535] = 47000
        - listen port: +range[1025..65535] = 47001
        - publish queue: mq.connection
        - listen queue: mq.connection
        - direct queues: map of +name, mq.connection
        - assignments: list of assignment
        - file batches: list of batch
        - dynamic batch: batch
        - active batch: batch +computed
        - heartbeats: map of +name, date
        
    update last heartbeat: func
        takes:
            - host: +name
        code:
            lock heartbeats:
                heartbeats[host] = date.now()
            
    get active batch: func
        returns:
            ab : batch = dynamic batch
        code:
            if file batches:
                ab = file batches[0]
        
    start publishing messages: func
        code:
            publish queue.connect(publish port, PUBLISH_MODE)
            
    start listening: func
        code:
            listen queue.connect(listen port, LISTEN_MODE, "domain.*", receive message)
            
    receive message: func
        takes:
            - msg: mq.message
        code:
            update last heartbeat(msg.from)
            
            # If all the functions below had the same signature, and if all of
            ... them had a reliable mapping between constant and function name,
            ... then we could do the following instead of using "handle":
            ...
            ... function namer: lambda code: `name of(code)`.lower().replace('_', '')
            ... dispatch(namer, arg1, arg2)
            handle msg.code:
                when it == ADD_BATCH:
                    add batch(msg.batch file path)
                when it == PAUSE_BATCH:
                    pause batch(msg.batch id)
                when it == ABANDON_BATCH:
                    abandon batch(msg.batch id)
                when it == FLUSH:
                    abandon batch(null)
                when it == TERMINATE:
                    terminate()
                when it == NEED_ASSIGNMENT:
                    assign(msg.from, msg.proposed item count)
                    
    assign: func
        takes:
            - requester host name: +name
            - proposed item count: int
        code:
            assignment := make assignment(requester host name, proposed item count)
            response := make message({
                topic := "domain.nitro.assignment"
                to := requester host name
            })
            if assignment:
                response.extend({
                    code := NITRO_NEW_ASSIGNMENT
                    payload := assignment
                })
            else:
                response.extend({
                    code := NITRO_ASSIGNMENT_NOT_AVAILABLE
                })
            response queue := get response queue for host(requester host name)
            response queue.send(msg)
            
    make assignment: func
        takes:
            *assign.params
        returns:
            - new: assignment
        code:
            commands := list of str
            batch := active batch
            lock batch:
                while len(commands) < proposed item count:
                    cmd := batch.next()
                    if cmd:
                        commands.append(cmd)
                    else:
                        break
            if commands:
                new := make assignment(batch.id, commands)
    
    add batch: func
        takes:
            - batch file path: str
        returns:
            - batch
        code:
            batch := make batch(batch file path)
            if batch:
                lock file batches:
                    file batches.append(batch)
                    
    enroll helpers: func
        code:
            # It's only legal to call this function if we haven't already
            ... enrolled a bunch of helpers.
            precondition: len(heartbeats) == 0
            msg = make message({
                topic := "domain.nitro.enroll",
                to := "*",
                code := NITRO_HELP_REQUESTED,
                nodes requested := helpers
            })
            mq.multicast(msg)
            
            # Sleep for a second to let responses come back -- but interrupt
            ... if we get word that all helpers responded.
            sleep(1000\millisecs, finished signal)
            if not finished signal:
                for node name in helpers:
                    if node not in heartbeats:
                        queue := 
            
    flush: func
        code:
            lock file batches:
                for batch in file batches:
                    abandon assignments for batch(batch)
                file batches.clear()
    run: func
        takes:
            - batches: list of batch
            - helpers: list of str
        code:
            start publishing messages()
            start listening()
            enroll helpers()
            start maintenance thread()
