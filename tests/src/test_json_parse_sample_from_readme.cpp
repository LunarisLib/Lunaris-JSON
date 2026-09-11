#include <Lunaris/json.h>

#include <thread>
#include <iostream>
#include <cmath>

#include "memory_check.h"

using namespace Lunaris::JSON;

int main() {
    // for debugging
    const auto state_before = mem_state();
    
    {
        const char buf[] = R"(
        {     "test"    :    10.58e96    ,  "hex_vals": [ 0x8886,
                0xFFFF,-12345,1.235e6  ,{"a":0,"b":null,"c":true,"str":"this is a value"}   ,9.9999E109 ],   
        "opbjecx":  {
                "innit"   :  "yee"  ,
        "totally_null_and_condensed":null
            }    }
        )";

        Json j{ std::make_shared<ParseableArrayWrapper>(buf, std::size(buf)) };

        if (j.get_type() != Json::e_type::OBJECT) return 1;
        if (j["test"].get_type() != Json::e_type::NUMBER) return 1;
        if (fabs(j["test"].get_double() - 10.58e96) > 1e-6) return 2;
        if (j["hex_vals"].get_type() != Json::e_type::ARRAY) return 1;
        if (j["hex_vals"][0].get_type() != Json::e_type::NUMBER) return 1;
        if (j["hex_vals"][0].get_uint() != 0x8886) return 2;
        if (j["hex_vals"][1].get_type() != Json::e_type::NUMBER) return 1;
        if (j["hex_vals"][1].get_uint() != 0xFFFF) return 2;
        if (j["hex_vals"][2].get_type() != Json::e_type::NUMBER) return 1;
        if (j["hex_vals"][2].get_uint() != -12345) return 2;
        if (j["hex_vals"][3].get_type() != Json::e_type::NUMBER) return 1;
        if (fabs(j["hex_vals"][3].get_double() - 1.235e6) > 1e-6) return 2;
        if (j["hex_vals"][4].get_type() != Json::e_type::OBJECT) return 1;
        if (j["hex_vals"][4]["a"].get_type() != Json::e_type::NUMBER) return 1;
        if (j["hex_vals"][4]["a"].get_int() != 0) return 2;
        if (j["hex_vals"][4]["b"].get_type() != Json::e_type::NIL) return 1;
        if (j["hex_vals"][4]["c"].get_type() != Json::e_type::BOOL) return 1;
        if (j["hex_vals"][4]["c"].get_bool() != true) return 2;
        if (j["hex_vals"][4]["str"].get_type() != Json::e_type::STRING) return 1;
        if (strcmp(j["hex_vals"][4]["str"].get_cstr(), "this is a value") != 0) return 2;
        if (j["hex_vals"][5].get_type() != Json::e_type::NUMBER) return 1;
        if (fabs(j["hex_vals"][5].get_double() - 9.9999E109) > 1e-6) return 2;
        if (j["opbjecx"].get_type() != Json::e_type::OBJECT) return 1;
        if (j["opbjecx"]["innit"].get_type() != Json::e_type::STRING) return 1;
        if (j["opbjecx"]["innit"].get_string() != "yee") return 2;
        if (j["opbjecx"]["totally_null_and_condensed"].get_type() != Json::e_type::NIL) return 1;

        j.print([](char c) {
            std::cout << c << std::flush;
        });
    }
    // for debugging
    const auto state_after = mem_state();

    return 0;
}