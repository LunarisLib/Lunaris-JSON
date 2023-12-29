#pragma once

#include <stdint.h>
#include <string.h>
#include <type_traits>

namespace Lunaris {
	// with engineer format support (+ decimal calculation if needed, not in power)
	template<typename T, typename Tprec = int16_t, typename std::enable_if<!std::is_floating_point<T>::value, int>::type = 0>
	T strtoT_e(const char*, const char** = nullptr);
	// double doesn't allow ~= operator, so we're doing just it = -it when negative!
	template<typename T, typename Tprec = int16_t, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
	T strtoT_e(const char*, const char** = nullptr);

	// any type from 0x, 0X or directly 0123ABC hex decl
	template<typename T, typename Tprec = int16_t, typename std::enable_if<!std::is_floating_point<T>::value, int>::type = 0>
	T hextoT(const char*, const char** = nullptr);
	// adapt to use int64_t and cast back. That's what we have for today.
	template<typename T, typename Tprec = int16_t, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
	T hextoT(const char*, const char** = nullptr);

	// auto select hextoT or strtoT based on input
	template<typename T, typename Tprec = int16_t>
	T autostrtoT(const char*, const char** = nullptr);

	class JSON {
	public:
		typedef void(*printer_char_function)(char);

		enum class type : uint16_t {
			INVALID,	// got bad or null
			BOOL,		// true or false, no child
			NIL,		// null, no child
			NUMBER,		// double or int, no child
			STRING,		// char array, no child
			ARRAY,		// array of refs. They are at *child and child has next...
			OBJECT		// object with stuff. They are at *child and child may have next or child...
		};
	private:
		struct ref {
			ref* next = nullptr; // alloc here
			/* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
			ref* child = nullptr; // alloc here
			type self_type = type::INVALID;

			const char* key_ptr = nullptr;
			const char* val_ptr = nullptr;

			ref* make_child();
			ref* make_next();

			void free_next_and_child();
		};

		struct nav {
			const char* buf = nullptr;
			size_t off = 0;

			const char* curr_off() const;
			void skip_next_spaces_auto();
			void skip_string_auto_escape();
			void skip_number();
			bool eof() const;
		};

		struct prt {
			printer_char_function fun;
			size_t lining; // 0 == no break no line, 1 = break, 1 space per depth, 2 = break, 2 ...
			size_t curr_depth = 0;
			char space_char = ' '; // can be \t if you need

			char* target = nullptr;
			size_t target_off = 0;

			void put(char);
		};

		ref* m_ref = nullptr;
		mutable char* m_charptr_clean = nullptr; // used temporarily in get_key or get_string so no extra data is returned!
		const bool m_root = true;

		// free all
		void _free();

		// copy and assume it is not root
		JSON(ref*);

		static void parse_value(ref*, nav*);

		static void parse_object(ref*, nav*);
		static void parse_array(ref*, nav*);

		static size_t print_any(ref*, prt&);
	public:
		// note: DO NOT DELETE CHAR*! IT IS USED INTERNALLY HERE!
		JSON(const char*, size_t = 0);
		JSON(const JSON&) = delete;
		void operator=(const JSON&) = delete;
		void operator=(JSON&&) = delete;
		JSON(JSON&&);
		~JSON();

		type get_type() const;

		int64_t get_int() const;
		uint64_t get_uint() const;
		float get_float() const;
		double get_double() const;
		bool get_bool() const;
		bool get_is_null() const;
		template<typename T> T get_number() const;

		const char* get_string_noalloc() const;
		const char* get_string() const;
		const char* get_key_noalloc() const;
		const char* get_key() const;

		size_t print(printer_char_function, const size_t = 2, const char = ' ') const;
		size_t print_to(char*, const size_t = 2, const char = ' ') const;

		JSON operator[](const char*) const;
		JSON operator[](size_t) const;
		JSON operator[](int) const;

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
	};
}

#include "json.ipp"