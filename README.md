#  README

Zen DSP lib

This is a lightweight audio processing library intended to run on embedded device and made in a way so it is compatible for incorporating in to Plugin/Standalone desktop/mobile projects as well 

To run this code you must include "zen.h" and create audio instance from "AudioInstance.h" that was included by zen.h




## Naming conventions

- Variable and method names are written with camel-case, and always begin with a lower-case letter, e.g. myVariableName
- Class names are also written in camel-case, but always begin with a capital letter, e.g. MyClassName
- Hungarian notation is not allowed. 
- private class/variables have trailing underscore eg MyPrivateVariable_ 
- Avoid using underscores in variable or method names. Use underscores for long_and_otherwise_hard_readable_names or to separate physical dimension of variable (delayTime_ms vs delayTime_us)
- Leading underscores have a special status for use in standard library code, so to use them in use code looks quite jarring.
- If you really have to write a macro, it must be ALL_CAPS_WITH_UNDERSCORES.
Since macros have no namespaces, their names must be guaranteed not to clash with macros or symbols used in other libraries or 3rd party code, so you should start them with something unique to your project. All the ZEN macros begin with ZEN_. 
For enums, use ALL_CAPS_WITH_UNDERSCORES with the same capitalisation that you'd use for a class and its member variables, e.g. Always place ENUM_NUM at the end which will store number of elements in enum:
```
enum class MyEnum
{
ENUM_VAL_1 = 0,
ENUM_VAL_2,  
ENUM_NUM
};
```


## Contributing

Collaborating is welcomed. If you want to make your contribution - fork the repo, make changes and create a Pull request. 
