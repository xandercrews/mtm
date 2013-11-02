calculated: mark
    implies:
        - type == str
    forces:
        - marks.append(-multiline)
    make: ctor +copyargs
        