#include <Lunaris/json.h>

#include <thread>
#include <iostream>

using namespace Lunaris::JSON;

int main() {
    const char buf[] = R"(
    {     "test"    :    10.58e96    ,  "hex_vals": [ 0x8886,
            0xFFFF,12345,1.235e6  ,{"a":0,"b":null,"c":true,"str":"this is a value"}   ,9.9999E109 ],   
	   "opbjecx":  {
	        "innit"   :  "yee"  ,
	   "totally_null_and_condensed":null
	    }    }
    )";

    Json j{ new ParseableArrayWrapper(buf, std::size(buf)) };

    if (j.get_type() != Json::e_type::OBJECT) return 1;
    if (j["test"].get_type() != Json::e_type::NUMBER) return 1;
    if (j["hex_vals"].get_type() != Json::e_type::ARRAY) return 1;
    if (j["hex_vals"][0].get_type() != Json::e_type::NUMBER) return 1;
    if (j["hex_vals"][1].get_type() != Json::e_type::NUMBER) return 1;
    if (j["hex_vals"][2].get_type() != Json::e_type::NUMBER) return 1;
    if (j["hex_vals"][3].get_type() != Json::e_type::NUMBER) return 1;
    if (j["hex_vals"][4].get_type() != Json::e_type::OBJECT) return 1;
    if (j["hex_vals"][4]["a"].get_type() != Json::e_type::NUMBER) return 1;
    if (j["hex_vals"][4]["b"].get_type() != Json::e_type::NIL) return 1;
    if (j["hex_vals"][4]["c"].get_type() != Json::e_type::BOOL) return 1;
    if (j["hex_vals"][4]["str"].get_type() != Json::e_type::STRING) return 1;
    if (j["hex_vals"][5].get_type() != Json::e_type::NUMBER) return 1;
    if (j["opbjecx"].get_type() != Json::e_type::OBJECT) return 1;
    if (j["opbjecx"]["innit"].get_type() != Json::e_type::STRING) return 1;
    if (j["opbjecx"]["totally_null_and_condensed"].get_type() != Json::e_type::NIL) return 1;

    j.print([](char c) {
        std::cout << c << std::flush;
    });

    return 0;
}