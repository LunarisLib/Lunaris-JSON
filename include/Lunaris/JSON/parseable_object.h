#pragma once

#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <fstream>

namespace Lunaris {
namespace JSON {

    class ParseableJson {
    public:
        ParseableJson() = default;
        virtual ~ParseableJson();

		ParseableJson(const ParseableJson&) = delete;
		void operator=(const ParseableJson&) = delete;

		virtual char get(const size_t at) const = 0;
		virtual void read(char* ptr, const size_t len, const size_t at) const = 0;
		virtual size_t max_off() const = 0;
    };

    class ParseableJsonFile : public ParseableJson {
    public:
        ParseableJsonFile(FILE*&& fp);
        ~ParseableJsonFile();

        ParseableJsonFile(ParseableJsonFile&& oth);
        void operator=(ParseableJsonFile&& oth);

        char get(const size_t at) const;
        void read(char* ptr, const size_t len, const size_t at) const;
        size_t max_off() const;
    private:
        FILE* m_fp;
    };

    class ParseableJsonFstream : public ParseableJson {
    public:
        ParseableJsonFstream(std::fstream&& fp);
        ~ParseableJsonFstream();

        ParseableJsonFstream(ParseableJsonFstream&& oth);
        void operator=(ParseableJsonFstream&& oth);

        char get(const size_t at) const;
        void read(char* ptr, const size_t len, const size_t at) const;
        size_t max_off() const;
    private:
        mutable std::fstream m_fp;
    };

    class ParseableArrayWrapper : public ParseableJson {
    public:
        ParseableArrayWrapper(const char* data, const size_t len);
        ~ParseableArrayWrapper();

        ParseableArrayWrapper(ParseableArrayWrapper&& oth);
        void operator=(ParseableArrayWrapper&& oth);

        char get(const size_t at) const;
        void read(char* ptr, const size_t len, const size_t at) const;
        size_t max_off() const;
    private:
        const char* m_data;
        size_t m_size;
    };

} // namespace JSON
} // namespace Lunaris