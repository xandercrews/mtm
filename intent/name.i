name: mark
    properties:
        - valid size: range of int
    implies type: str
    implies marks: -multiline
    make: ctor +copyargs
        