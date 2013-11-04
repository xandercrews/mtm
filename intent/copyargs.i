copyargs: mark
    properties:
        - params to copy: list of param def
        - properties to init: list of property def
        
    # Override the binds method of the mark class.
    can bind: mark.can bind
        # The arrow operator is like C++'s dynamic cast, and the .? operator
        ... is a "try to traverse this relationship but return null and stop
        ... without an exception if the traversal yields null" operator.
        params to copy = (site->func).?takes
        properties to init = (site->func).?associated class.?properties
        answer = properties to init->bool 
                        
    generate: mark.generate
        insert point := mark.insertion points.after preconditions
        code:
            if params to copy:
                for param def in params to copy:
                    name := param def.name
                    match := find(properties to init, lambda x: x.name == name)
                    if match:
                        generated code += "this.{name} = {name}\n"
            else:
                for property def in properties to init:
                    generated code += "this.{name} = {name}\n"