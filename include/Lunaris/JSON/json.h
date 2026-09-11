#pragma once

#include <Lunaris/JSON/parseable_object.h>

#include <cstdint>
#include <memory>
#include <type_traits>
#include <functional>

namespace Lunaris {
namespace JSON {

    class Json {
    public:
		typedef void(*printer_char_function)(char);

		enum class e_type : uint8_t {
			INVALID,	// got bad or null
			BOOL,		// true or false, no child
			NIL,		// null, no child
			NUMBER,		// double or int, no child
			STRING,		// char array, no child
			ARRAY,		// array of refs. They are at *child and child has next...
			OBJECT		// object with stuff. They are at *child and child may have next or child...
		};

        Json(const Json&) = delete;
        void operator=(const Json&) = delete;
        void operator=(Json&& oth) = delete;

        Json(ParseableJson*&& readable_json);
        Json(Json&& oth);

        ~Json();

        e_type get_type() const;

		int64_t get_int() const;
		uint64_t get_uint() const;
		float get_float() const;
		double get_double() const;
		bool get_bool() const;
		bool get_is_null() const;

		template<typename T>
        T get_number() const;

		const char* get_string() const;
		const char* get_key() const;

		size_t print(printer_char_function f, const size_t lining = 2, const char space_ch = ' ') const;
		size_t print_to(char* buf, const size_t lining = 2, const char space_ch = ' ') const;

		Json operator[](const char* key) const;
		Json operator[](size_t idx) const;
		Json operator[](int idx) const;

		operator int16_t() const;
		operator int32_t() const;
		operator int64_t() const;
		operator uint16_t() const;
		operator uint32_t() const;
		operator uint64_t() const;
		operator bool() const;
		operator float() const;
		operator double() const;
		operator const char*() const;

    private:
        // with engineer format support (+ decimal calculation if needed, not in power)
        template<typename T, typename std::enable_if<!std::is_floating_point<T>::value, int>::type = 0>
        T strtoT_e(const ParseableJson* const, const size_t) const;
        // double doesn't allow ~= operator, so we're doing just it = -it when negative!
        template<typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
        T strtoT_e(const ParseableJson* const, const size_t) const;

        // any type from 0x, 0X or directly 0123ABC hex decl
        template<typename T, typename std::enable_if<!std::is_floating_point<T>::value, int>::type = 0>
        T hextoT(const ParseableJson* const, size_t) const;
        // adapt to use int64_t and cast back. That's what we have for today.
        template<typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
        T hextoT(const ParseableJson* const, size_t) const;

        // auto select hextoT or strtoT based on input
        template<typename T>
        T autostrtoT(const ParseableJson* const, const size_t) const;


		struct ref {
			ref* next = nullptr; // alloc here
			/* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
			ref* child = nullptr; // alloc here

			size_t key_ptr{};

			e_type self_type = e_type::INVALID;
			bool key_is_val = false;

			ref* make_child();
			ref* make_next();

			void free_next_and_child();

			size_t get_val_ptr(const ParseableJson* const) const; // run key_ptr + ...: * if bool, nil, number or string only

			// checks within valid range
			bool is_eq_key_ptr_val(const ParseableJson* const, const char*, const size_t) const;
			// no check for range
			bool is_eq_val_ptr_val(const ParseableJson* const, const char*, const size_t) const;
		};

		struct nav {
			//const char* buf = nullptr;
			const ParseableJson* const base;
			size_t off;
			mutable char minibuf[8]{};

			char curr_ch() const;
			const char* curr_off() const;
			void skip_next_spaces_auto();
			void skip_string_auto_escape();
			void skip_number();
			bool eof() const;
		};

		struct prt {
			const ParseableJson* const m_base = nullptr; // source
			printer_char_function fun;
			size_t lining; // 0 == no break no line, 1 = break, 1 space per depth, 2 = break, 2 ...
			size_t curr_depth = 0;
			char space_char = ' '; // can be \t if you need

			char* target = nullptr;
			size_t target_off = 0;

			void put(char);
		};

		ref* m_ref{};
		mutable std::unique_ptr<char[]> m_charptr_clean; // used temporarily in get_key or get_string so no extra data is returned!
		const bool m_root;
		ParseableJson* m_base; // source

		// free all
		void _free();

		// copy and assume it is not root
		Json(ref*, ParseableJson*);

		char get_val_of(ref*);

		static void parse_value(ref*, nav*);

		static void parse_object(ref*, nav*);
		static void parse_array(ref*, nav*);

		static size_t print_any(ref*, prt&);
    };

} // namespace JSON
} // namespace Lunaris

#include <Lunaris/JSON/impl/json.ipp>