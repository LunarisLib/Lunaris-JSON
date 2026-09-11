#pragma once

#include <Lunaris/JSON/parseable_object.h>

#include <cstdint>
#include <memory>
#include <type_traits>
#include <functional>
#include <string>

namespace Lunaris {
namespace JSON {

	/**
	 * @brief Json is the object to parse and manage Json objects
	 */
    class Json {
    public:
		typedef void(*printer_char_function)(char);

		/**
		 * @brief Type of the object stored in this level of the Json file.
		 */
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

		/**
		 * @brief Construct a new Json object from a Parseable Json
		 * 
		 * You can make your own implementation of ParseableJson if you want to too.
		 * 
		 * @param `readable_json` a ParseableJson to be managed by this object
		 */
        Json(std::shared_ptr<ParseableJson> readable_json);

		/**
		 * @brief Construct a new Json object moving another to it
		 * 
		 * @param `oth` the Json object being moved
		 */
        Json(Json&& oth);

        ~Json();

		/**
		 * @brief Get the type this object is holding
		 * 
		 * @return `e_type` enum with the type of this object
		 */
        e_type get_type() const;

		/**
		 * @brief Get as int64_t
		 * 
		 * @return `int64_t` the value as int64_t, or zero
		 */
		int64_t get_int() const;

		/**
		 * @brief Get as uint64_t
		 * 
		 * @return `uint64_t` the value as int64_t, or zero
		 */
		uint64_t get_uint() const;

		/**
		 * @brief Get as float
		 * 
		 * @return `float` the value as float, or zero
		 */
		float get_float() const;

		/**
		 * @brief Get as double
		 * 
		 * @return `double` the value as double, or zero
		 */
		double get_double() const;

		/**
		 * @brief Get as bool
		 * 
		 * @return `bool` the value as bool, or zero
		 */
		bool get_bool() const;

		/**
		 * @brief Get if value is null
		 * 
		 * @return `bool` is the value null?
		 */
		bool get_is_null() const;

		/**
		 * @brief Get as close as possible to type T (numeric)
		 * 
		 * @return `T` 
		 */
		template<typename T>
        T get_number() const;

		/**
		 * @brief Get value as string
		 * 
		 * @return `std::string` copy of value as string
		 */
		std::string get_string() const;

		/**
		 * @brief Get value as C string
		 * 
		 * @return `const char*` temporary array with value, valid until another get_* call
		 */
		const char* get_cstr() const;

		/**
		 * @brief Get key as string
		 * 
		 * @return `const char*` temporary array with key, valid until another get_* call
		 */
		const char* get_key() const;

		/**
		 * @brief Do a dump of the object using the callback for print
		 * 
		 * @param `f` callback that receives each `char`
		 * @param `lining` each line gap/offset
		 * @param `space_ch` what to use as space
		 * @return `size_t` the amount of characters printed
		 */
		size_t print(printer_char_function f, const size_t lining = 2, const char space_ch = ' ') const;

		/**
		 * @brief Do a dump of the object on the `buf` buffer
		 * 
		 * @param `buf` the buffer to write into, or null to calculate size
		 * @param `lining` each line gap/offset
		 * @param `space_ch` what to use as space
		 * @return `size_t` the amount of characters written
		 */
		size_t print_to(char* buf, const size_t lining = 2, const char space_ch = ' ') const;

		/**
		 * @brief Access object's value from key
		 * 
		 * @param `key` the key being accessed
		 * @return `Json` the object in this key
		 */
		Json operator[](const char* key) const;

		/**
		 * @brief Access an index of the array
		 * 
		 * @param `idx` the index, from 0 to size of it
		 * @return `Json` the object in this index
		 */
		Json operator[](size_t idx) const;

		/**
		 * @brief Access an index of the array
		 * 
		 * @param `idx` the index, from 0 to size of it
		 * @return `Json` the object in this index
		 */
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
		operator std::string() const;

    private:
        // with engineer format support (+ decimal calculation if needed, not in power)
        template<typename T, typename std::enable_if<!std::is_floating_point<T>::value, int>::type = 0>
        T strtoT_e(const std::shared_ptr<ParseableJson>&, const size_t) const;
        // double doesn't allow ~= operator, so we're doing just it = -it when negative!
        template<typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
        T strtoT_e(const std::shared_ptr<ParseableJson>&, const size_t) const;

        // any type from 0x, 0X or directly 0123ABC hex decl
        template<typename T, typename std::enable_if<!std::is_floating_point<T>::value, int>::type = 0>
        T hextoT(const std::shared_ptr<ParseableJson>&, size_t) const;
        // adapt to use int64_t and cast back. That's what we have for today.
        template<typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
        T hextoT(const std::shared_ptr<ParseableJson>&, size_t) const;

        // auto select hextoT or strtoT based on input
        template<typename T>
        T autostrtoT(const std::shared_ptr<ParseableJson>&, const size_t) const;


		struct ref {
			std::shared_ptr<ref> next; // alloc here
			/* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
			std::shared_ptr<ref> child; // alloc here

			size_t key_ptr{};

			e_type self_type = e_type::INVALID;
			bool key_is_val = false;

			std::shared_ptr<ref> make_child();
			std::shared_ptr<ref> make_next();

			void free_next_and_child();

			size_t get_val_ptr(const std::shared_ptr<ParseableJson>&) const; // run key_ptr + ...: * if bool, nil, number or string only

			// checks within valid range
			bool is_eq_key_ptr_val(const std::shared_ptr<ParseableJson>&, const char*, const size_t) const;
			// no check for range
			bool is_eq_val_ptr_val(const std::shared_ptr<ParseableJson>&, const char*, const size_t) const;
		};

		struct nav {
			//const char* buf = nullptr;
			const std::shared_ptr<ParseableJson>& base;
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
			const std::shared_ptr<ParseableJson>& m_base; // source
			printer_char_function fun;
			size_t lining; // 0 == no break no line, 1 = break, 1 space per depth, 2 = break, 2 ...
			size_t curr_depth = 0;
			char space_char = ' '; // can be \t if you need

			char* target = nullptr;
			size_t target_off = 0;

			void put(char);
		};

		std::shared_ptr<ref> m_ref{};
		mutable std::unique_ptr<char[]> m_charptr_clean; // used temporarily in get_key or get_string so no extra data is returned!
		const bool m_root;
		std::shared_ptr<ParseableJson> m_base; // source

		// free all
		void _free();

		// copy and assume it is not root
		Json(std::shared_ptr<ref>, std::shared_ptr<ParseableJson>);

		char get_val_of(std::shared_ptr<ref>&);

		static void parse_value(std::shared_ptr<ref>&, nav&);

		static void parse_object(std::shared_ptr<ref>&, nav&);
		static void parse_array(std::shared_ptr<ref>&, nav&);

		static size_t print_any(std::shared_ptr<ref>, prt&);
    };

} // namespace JSON
} // namespace Lunaris

#include <Lunaris/JSON/impl/json.ipp>