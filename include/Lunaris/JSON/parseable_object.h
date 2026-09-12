#pragma once

#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <fstream>

namespace Lunaris {
namespace JSON {

    /**
     * @brief This is an interface to make Json parsing universal. You can do it in your own custom way, or use one of the implemented ones.
     */
    class ParseableJson {
    public:
        ParseableJson() = default;
        virtual ~ParseableJson();

		ParseableJson(const ParseableJson&) = delete;
		void operator=(const ParseableJson&) = delete;

        /**
         * @brief Read a character at a specific position
         * 
         * @param `at` the position, from 0 to max_off() 
         * @return `char` the character in that position in memory
         */
		virtual char get(const size_t at) const = 0;

        /**
         * @brief Read a block of data from a point to another
         * 
         * @param `ptr` the array to store the read data to
         * @param `len` the amount of bytes (max) to read from the memory
         * @param `at` the offset of the original data to read from
         */
		virtual void read(char* ptr, const size_t len, const size_t at) const = 0;

        /**
         * @brief Get the maximum size to read from this.
         * 
         * @return `size_t` expected size of this object or -1 for undefined
         */
		virtual size_t max_off() const = 0;
    };

    /**
     * @brief Implementation of ParseableJson for FILE*
     */
    class ParseableJsonFile : public ParseableJson {
    public:
        /**
         * @brief Construct a new Parseable Json File object from a FILE*
         * 
         * NOTE: This takes control of the FILE*, so you don't need to worry about closing it later with fclose!
         * 
         * @param `fp` the FILE* that will be absorbed by this
         */
        ParseableJsonFile(FILE*&& fp);
        ~ParseableJsonFile();

        ParseableJsonFile(ParseableJsonFile&& oth);
        void operator=(ParseableJsonFile&& oth);

        /**
         * @brief Read a character at a specific position
         * 
         * @param `at` the position, from 0 to max_off() 
         * @return `char` the character in that position in memory
         */
        char get(const size_t at) const;

        /**
         * @brief Read a block of data from a point to another
         * 
         * @param `ptr` the array to store the read data to
         * @param `len` the amount of bytes (max) to read from the memory
         * @param `at` the offset of the original data to read from
         */
        void read(char* ptr, const size_t len, const size_t at) const;

        /**
         * @brief Get the maximum size to read from this.
         * 
         * @return `size_t` expected size of this object or -1 for undefined
         */
        size_t max_off() const;
    private:
        FILE* m_fp;
    };

    /**
     * @brief Implementation of ParseableJson for std::fstream
     */
    class ParseableJsonFstream : public ParseableJson {
    public:
        /**
         * @brief Construct a new Parseable Json Fstream object from a std::fstream
         * 
         * NOTE: This takes control of the std::fstream, so you don't need to worry about managing it later!
         * 
         * @param `fp` the std::fstream that will be absorbed by this
         */
        ParseableJsonFstream(std::fstream&& fp);
        ~ParseableJsonFstream();

        ParseableJsonFstream(ParseableJsonFstream&& oth);
        void operator=(ParseableJsonFstream&& oth);

        /**
         * @brief Read a character at a specific position
         * 
         * @param `at` the position, from 0 to max_off() 
         * @return `char` the character in that position in memory
         */
        char get(const size_t at) const;

        /**
         * @brief Read a block of data from a point to another
         * 
         * @param `ptr` the array to store the read data to
         * @param `len` the amount of bytes (max) to read from the memory
         * @param `at` the offset of the original data to read from
         */
        void read(char* ptr, const size_t len, const size_t at) const;

        /**
         * @brief Get the maximum size to read from this.
         * 
         * @return `size_t` expected size of this object or -1 for undefined
         */
        size_t max_off() const;
    private:
        mutable std::fstream m_fp;
    };

    /**
     * @brief Implementation of ParseableJson for an char array
     */
    class ParseableArrayWrapper : public ParseableJson {
    public:
        /**
         * @brief Construct a new Parseable Array Wrapper object from an existing array of char
         * 
         * NOTE: This doesn't take the array, it only references it. You must be sure that the array never deallocates before this object!
         * 
         * @param `data` pointer to the array
         * @param `len` the array's length
         */
        ParseableArrayWrapper(const char* data, const size_t len);
        ~ParseableArrayWrapper();

        ParseableArrayWrapper(ParseableArrayWrapper&& oth);
        void operator=(ParseableArrayWrapper&& oth);

        /**
         * @brief Read a character at a specific position
         * 
         * @param `at` the position, from 0 to max_off() 
         * @return `char` the character in that position in memory
         */
        char get(const size_t at) const;

        /**
         * @brief Read a block of data from a point to another
         * 
         * @param `ptr` the array to store the read data to
         * @param `len` the amount of bytes (max) to read from the memory
         * @param `at` the offset of the original data to read from
         */
        void read(char* ptr, const size_t len, const size_t at) const;

        /**
         * @brief Get the maximum size to read from this.
         * 
         * @return `size_t` expected size of this object or -1 for undefined
         */
        size_t max_off() const;
    private:
        const char* m_data;
        size_t m_size;
    };

} // namespace JSON
} // namespace Lunaris