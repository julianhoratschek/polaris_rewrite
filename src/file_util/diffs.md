# CMD-Files

- Boolean processing:
    - Legacy:
        1 == True, Anything else == False
    - Rewrite:
        - Make booleans correct
        0 == False, Anything else == True
- common commands:
    - Legacy:
        - Could stand everywhere, are pre-processed
    - Rewrite:
        - Legacy method is error-prone, as multiple locations could define
          globally used variables
        - common commands can only be defined ONCE
- Parser Errors:
    - Legacy
        - Mostly mute errors, still true result after parsing, ignoring
          wrong input parameters
    - Rewrite
        - Don't accept errors in cmd file, as it might point towards
          more errors and will result in wrong outcome
        - Errors are reported and marked
- Line Numbers:
    - Legacy
        - Does not count unparsed lines
    - Rewrite
        - Reports line- and column numbers of Errors
        - Reports correct line number of errors
- Unused tasks
    - Legacy
        - Reads and stores Data of unused tasks
    - Rewrite
        - Does not process unused tasks, does not load data

