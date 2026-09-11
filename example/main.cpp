#include <Lunaris/json.h>

using namespace Lunaris::JSON;

int main() {
    const char buf[] = R"(
    {     "test"    :    10.58e96    ,  "hex_vals": [ 0x8886,   0xFFFF,12345,1.235e6  ,{"a":0,"b":null,"c":true,"str":"this is a value"}   ,9.9999E109 ],   
	   "opbjecx":  {
	        "innit"   :  "yee"  ,
	   "totally_null_and_condensed":null
	    }    }
    )";

    Json j{ new ParseableArrayWrapper(buf, std::size(buf)) };

    j.print(
        [](char c){putchar(c);}, 4, ' '
    );

    return 0;
}