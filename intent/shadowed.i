| A +shadowed property is one that has a public getter and (optional) setter,
... but that is backed by some kind of internal storage.
shadowed: mark

    shared once:
        mark co-occurrence rules:
            # Whenever a noun is shadowed, it must be calculated as well, to
            ... whatever level of explicitness is claimed for shadowed.
            - {+calculated, claim.affirm, claim.affirm}
            - {+calculated, claim.implied, claim.implied}
        type co-occurrence rules:
            # Require that this mark be applied only to properties.
            - {property, claim.affirm, claim.affirm}
    
    properties:
        - type
        
    make: +copyargs ctor
        takes: - type
 