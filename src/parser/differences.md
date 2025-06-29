# Differences between old and new cmd parser/interpreter

- Common-Blocks **MUST** be located at the top of the file.
    - Blocks are processed in order of definition
    - This means, all Task-Blocks preceding a Common-Block will not 
      use common-parameters defined in said Common-Block
- Boolean values are "false" with value 0 and true with any other numerical
  value
    - Before, Boolean values were "true" with value 1 and false with any
      other value
- ISRF source method works slightly different
    - It now will always set a path, if defined.
    - Before, path was ignored - even when set - if 2 numerical parameters were given
- Detectors are a bit less strict
    - Missing values are mostly filled in wich 0.
    - Before, only selected parameter-combinations were allowed
