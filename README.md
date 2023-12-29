# Lunaris JSON! in C++!

*Of course it is in C++, but why? Why did I create this?*

*Look at this fancy code:*

```cpp
#include <string>
#include "json.h"

// ...
  const std::string test = 
	"   {     \"test\"    :    10.58e96    ,  \"hex_vals\": [ 0x8886,   0xFFFF,12345,1.235e6  ,{\"a\":0,\"b\":null,\"c\":true,\"str\":\"this is a value\"}   ,9.9999E109 ],    "
	"   \"opbjecx\":  {"
	"  \"innit\"   :  \"yee\"  ,"
	  "\"totally_null_and_condensed\":null"
	"  }    } ";

  JSON j(test.c_str(), test.size());
// ...
```
*What do you see? How much memory does it allocate for this? The answer: probably less than many (if not all) libraries around here. Why? Because values are stored as pointers. Everything in it is pointers. That's the only downside: DO NOT DEALLOCATE WHAT YOU'VE PASSED TO IT!*

*So, how is its structure?*

```cpp
struct ref {
  ref* next; // if this is an array or sequence of objects in an object, this is the next one
  ref* child; // if this is a key -> object or key -> array, the object/array is here
  type type; // its type, like NUMBER, STRING, OBJECT...

  const char* key_ptr; // start of key, if it makes sense, pointer from parse
  const char* val_ptr; // start of value, if it is a kind that has one, pointer from parse
};
```

*4 pointers and one 16 bit integer enum. Good enough? I think so. Better than saving an int, a double, something else and a copy of a string.*

*I'm doing this reference thing quick so you can use it too, but maybe in the future I can make this readme better.*

*Examples of use: (assuming you've already JSON(string...) things up):*

```cpp
void putchar_ch(char ch) { putchar(ch); } // make putchar putchar(char)
// ... in a func:
const size_t printed_chars = j.print(putchar_ch, 4, ' '); // putchar_ch is called, things are printed in cmd.
char* buff = new char[printed_chars + 1] {'\0'};
j.print_to(buff, 4, ' ');
// buff has all the data formatted! Use it as you wish. Don't forget to:
delete[] buff;
```
```cpp
JSON c = j["hex_vals"][4]["str"];
const char* c_str = c;
std::cout << "Yooo: " << c_str << std::endl;
std::cout << "Also works: " << (const char*)c << std::endl;
// actually you can cast to a lot of types and it should work. If you want to be sure, use the get_*** ones.
```
