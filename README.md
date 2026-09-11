# Lunaris JSON Library

This is a JSON library that you can add on your project! It works on both Windows and Linux.

There are some tests to check if it builds correctly and they can be disabled with `BUILD_TESTS OFF`.

## How to add the project to your project

### Using FetchContent

You can create a file like `cmake/installLibrary.cmake` and put in there:

```cmake
include(FetchContent)

FetchContent_Declare(
    lunaris-json
    GIT_REPOSITORY https://github.com/LunarisLib/lunaris-json.git
    GIT_TAG        (put version here)
)
FetchContent_MakeAvailable(lunaris-json)
```

This will allow you to download and link the library like:

```cmake
# ...

include(cmake/installLibrary.cmake) # does the FetchContent

target_link_libraries(YourProjectName PRIVATE
    lunaris::lunaris-json
)
```

### Using find_package()

If you get the install version with the lib and headers and want to avoid recompiling the library yourself, you can do

```cmake
# ...

find_package(lunaris-json REQUIRED)

target_link_libraries(YourProjectName PRIVATE
    lunaris::lunaris-json
)
```

The find_package will try to find the `lunaris-json-config.cmake` or similar files that should be available to download in the Release tab.

## How to use it

Look at this fancy code:

```cpp
// ...
#include <Lunaris/json.h>

using namespace Lunaris::JSON;

// ...
const char buf[] = R"(
  {     "test"    :    10.58e96    ,  "hex_vals": [ 0x8886,
          0xFFFF,12345,1.235e6  ,{"a":0,"b":null,"c":true,"str":"this is a value"}   ,9.9999E109 ],   
    "opbjecx":  {
        "innit"   :  "yee"  ,
    "totally_null_and_condensed":null
    }    }
  )";

Json j{ std::make_shared<ParseableArrayWrapper>(buf, std::size(buf)) };
// ...
```

What do you see? How much memory does it allocate for this? The answer: probably less than many (if not all) libraries around here. Why? Because values are stored as pointers. Everything in it is a pointer. That's the only downside: DO NOT DEALLOCATE WHAT YOU'VE PASSED TO IT!

### So, how is its structure?

```cpp
struct ref {
  ref* next; // if this is an array or sequence of objects in an object, this is the next one
  ref* child; // if this is a key -> object or key -> array, the object/array is here

  size_t key_ptr; // start of key or value, offset from parse
  
  type self_type; // its type, like NUMBER, STRING, OBJECT...
  bool key_is_val; // used when it is in an array.
};
```

2 pointers, one offset, one 8 bit integer enum and a bool. Good enough? I think so. Better than saving an int, a double, something else and a copy of a string.

I'm doing this reference thing quick so you can use it too, but maybe in the future I can make this readme better.

### Examples

```cpp
// using the first example...
// ... in a func:
const size_t printed_chars = j.print([](char c){putchar(c);}, 4, ' '); // putchar_ch is called, things are printed in cmd.
char* buff = new char[printed_chars + 1] {'\0'};
j.print_to(buff, 4, ' ');
// buff has all the data formatted! Use it as you wish. Don't forget to:
delete[] buff;
```
```cpp
// using the first example...
JSON c = j["hex_vals"][4]["str"];
const char* c_str = c;
std::cout << "Yooo: " << c_str << std::endl;
std::cout << "Also works: " << (const char*)c << std::endl;
// actually you can cast to a lot of types and it should work. If you want to be sure, use the get_*** ones.
```