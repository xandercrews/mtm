server engine: class

    properties:
        - assigned helpers: +assignonce list of +name
        - direct queues: map of +name to mq.connection
        - assignments: map of +name to assignment
        - file batches: list of batch
        - dynamic batch: batch
        - active batch: +calculated batch
        
    make: ctor
        takes:
            - *engine.takes
            - assigned helpers
            
    get active batch: func
        returns:
            ab : batch = dynamic batch
        code:
            if file batches:
                ab = file batches[0]
        
    receive message: engine.receive message
        takes:
            - msg: mq.message
        code:
            if engine.receive message(msg): return
            
            # If all the functions below had the same signature, and if all of
            ... them had a reliable mapping between constant and function name,
            ... then we could do the following instead of using "handle":
            ...
            ... function namer: lambda code: *(name of(code)).lower().replace('_', '')
            ... dispatch(namer, arg1, arg2)
            handle msg.code:
                # The "when xyz" is a short form of "when it == "
                when ADD_BATCH: add batch(msg.batch file path)
                when PAUSE_BATCH: pause batch(msg.batch id)
                when ABANDON_BATCH: abandon batch(msg.batch id)
                when FLUSH: abandon batch(null)
                when NEED_ASSIGNMENT: assign(msg.from, msg.proposed item count)
                when it == ACCEPT_ASSIGNMENT || it == REJECT_ASSIGNMENT:
                    finish enrolling(msg)
                    
    finish enrolling: func
        takes:
            - msg: mq.message
        code:
            lock assignments
            if msg.from in assignments: return
            
            
            
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
                nodes requested := assigned helpers
            })
            mq.multicast(msg)
            
            # Sleep for a second to let responses come back -- but interrupt
            ... if we get word that all helpers responded.
            if sleep(1000\millisecs, finished signal) == 0:
                for node name in helpers:
                    if node not in heartbeats:
                        send direct(node name, msg)
                        
                if sleep(1000\millisecs, finished signal) == 0:
                    log("Couldn't enroll everybody.")
                
            
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
