| Specifies that a string contains short, single-line identifiers.
name: mark
    properties:
        - valid size: range of int = [1..128]
    
    adjust marks: -multiline
    
    make: +copyargs +inheritargs ctor
        takes:
            - valid size
    
    can bind: make.can bind
        code:
            answer = type(site) is str
        