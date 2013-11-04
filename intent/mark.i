| Enriches semantic information about a class, function, parameter, variable,
... or other noun.
...
... Marks appear between the colon following a noun's name, and any data type
... or assignment. They must be preceded by "+" or "a".
mark: class
    | What type of claim is a mark making about the noun it modifies?
    claim: enum
        columns:
            - name: +name
            - number: int
            - prefix char: char
        rows:
            | The mark had a "+" prefix to affirm that its semantics obtain.
            - affirm | 2 | '+'
            
            | The mark had a "-" prefix to deny that its semantics obtain.
            - deny | -1 | '-'
            
            | The mark is relevant but nothing more specific is claimed.
            ... This is mainly useful in internal code that tests marks for
            ... mutual compatibility.
            - relevant | 0 | '?'
            
            | The mark's semantics are implied but not explicitly affirmed.
            - imply | 1 | '.'
        
        | Is this claim compatible with another claim?
        ... @affirm and @deny not mutually compatible; @relevant is compatible
        ... with any other claim.
        compatible with: func
            takes:
                - other claim: claim
            returns:
                answer: bool
            code:
                handle this:
                    when relevant: answer = true
                    when affirm, imply: answer = (other > deny)
                    when deny: answer = (other <= relevant)
            
    co-occurrence rule (crule): struct of T
        fields:
            | An object against which co-occurrence rules can be tested.
            - item: T
            
            | The claim that must be made by the mark that we're testing, in
            ... order for this rule to be applicable.
            - test claim := claim.relevant
            
            | If this rule applies, what do we know about the co-occurrence
            ... of item? By default, we deny co-occurrence, which makes the
            ... rule about incompatibility.
            - occurrence claim := claim.deny
            
    # Some generic typedefs
    crules of T: generator of crules of T
    crlist of T: list of crules of T
        
    properties:
        - claim
        - effective marks: crules of T
        
    shared properties:
        
        | Rules about other marks co-occurring with this one.
        - mark co-occurrence rules: +shadowed(crlist of mark) crules of mark
        
        | Rules about other types where this mark can or cannot apply.
        - type co-occurrence rules: +shadowed(crlist of type) crules of type
        
    get mark co-occurrence rules: getter
        code:
            yield this class
            for rule in explicit marks:
                yield rule.
            for rule in implied marks:
                
    make: ctor
        takes:
            - claim
        
    # This shows how you can declare params that can be used in multiple places.
    ... The base class "class" defines "takes" as a list of param def; here
    ... we define a new noun that's of that same type, so we can reuse it
    ... instead of naming and describing the semantics of parameters over and
    ... over again. It also illustrates two other syntactic features:
    ...
    ...   - Any container can be extended by giving the list name followed by a
    ...     colon, and then having a series of nouns, each preceded by "- ".
    ...   - The nouns can be singular or can themselves be containers.
    
    bind params: func.takes.type
        | A noun where a mark has been offered.
        - noun
        
    bind returns: nestable list of param def:
        | Describes why binding should not proceed. If null or empty, the mark
        ... should be bound to the noun.
        - fail reasons: list of str
        
    | Identify any reasons why this mark can't apply to nouns of the specified
    ... type.
    get reasons why incompatible: func
        takes: - bind params
        returns: - bind returns
        code:
            for rule in type compatibility rules:
            
                # Does the rule apply to this mark?
                if rule.test claim.compatible with(claim):
                
                    # Does the rule apply to this type of noun?
                    if noun.type is rule.item:
                        if rule.occurrence claim == claim.deny:
                            fail reasons ?+= "Mark {claim.prefix char}{this
                            ... class.name} is incompatible with
                            ... {rule.item.name}."
                            
                    else if rule.occurrence claim == claim.affirm:
                        fail reasons ?+= "Mark {claim.prefix char}{this
                        ... class.name} must be attached to nouns of type
                        ... \"{type.name}\"."
                            
            for rule in mark co-occurrence rules:
            
                # Only deny rules can be a reason for incompatibility...
                if rule.occurrence claim == claim.deny:
                
                    # Does the rule apply to this mark?
                    if rule.test claim.compatible with(claim):
                    
                        for mark in noun def.effective marks:
                        
            
    try bind: func
        takes: - bind params
        returns: - bind returns
        code:
            fail reasons ?+= get reasons why not eligible(site)
