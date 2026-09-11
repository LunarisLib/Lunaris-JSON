#include <cstring>

#include <Lunaris/JSON/parseable_object.h>

namespace Lunaris {
namespace JSON {
    
    ParseableJson::~ParseableJson() {        
    }


    ParseableJsonFile::ParseableJsonFile(FILE*&& fp)
        : m_fp(std::move(fp))
    {}

    ParseableJsonFile::~ParseableJsonFile() {
        if (m_fp) fclose(m_fp);
    }

    ParseableJsonFile::ParseableJsonFile(ParseableJsonFile&& oth)
        : m_fp(std::move(oth.m_fp))
    {
        oth.m_fp = nullptr;
    }

    void ParseableJsonFile::operator=(ParseableJsonFile&& oth) {
        if (m_fp) fclose(m_fp);
        m_fp = oth.m_fp;
        oth.m_fp = nullptr;
    }    

    char ParseableJsonFile::get(const size_t at) const {
#ifdef WIN32
        _fseeki64(m_fp, static_cast<int64_t>(at), SEEK_SET);
#else
        fseeko(m_fp, static_cast<int64_t>(at), SEEK_SET);
#endif
        fseek(m_fp, static_cast<long>(at), SEEK_SET);
        int v = fgetc(m_fp);
        return v < 0 ? '\0' : static_cast<char>(v);
    }

    void ParseableJsonFile::read(char* ptr, const size_t len, const size_t at) const {
#ifdef WIN32
        _fseeki64(m_fp, static_cast<int64_t>(at), SEEK_SET);
#else
        fseeko(m_fp, static_cast<int64_t>(at), SEEK_SET);
#endif
        fread(ptr, sizeof(char), len, m_fp);
    }

    size_t ParseableJsonFile::max_off() const {
        return static_cast<size_t>(-1);
    }


    ParseableJsonFstream::ParseableJsonFstream(std::fstream&& fp)
        : m_fp(std::move(fp))
    {}

    ParseableJsonFstream::~ParseableJsonFstream() {
        if (m_fp.is_open()) m_fp.close();
    }

    ParseableJsonFstream::ParseableJsonFstream(ParseableJsonFstream&& oth)
        : m_fp(std::move(oth.m_fp))
    {}

    void ParseableJsonFstream::operator=(ParseableJsonFstream&& oth) {
        if (m_fp.is_open()) m_fp.close();
        m_fp = std::move(oth.m_fp);
    }

    char ParseableJsonFstream::get(const size_t at) const {
        m_fp.seekg(at);
        int v = m_fp.get();
        return v < 0 ? '\0' : static_cast<char>(v);
    }
    
    void ParseableJsonFstream::read(char* ptr, const size_t len, const size_t at) const {
        m_fp.seekg(at);
        m_fp.read(ptr, len);
    }
    
    size_t ParseableJsonFstream::max_off() const {
        return static_cast<size_t>(-1);
    }


    ParseableArrayWrapper::ParseableArrayWrapper(const char* data, const size_t len)
        : m_data(data), m_size(len)
    {}

    ParseableArrayWrapper::~ParseableArrayWrapper() 
    {}

    ParseableArrayWrapper::ParseableArrayWrapper(ParseableArrayWrapper&& oth)
        : m_data(oth.m_data), m_size(oth.m_size)
    {
        oth.m_data = nullptr;
        oth.m_size = 0;
    }

    void ParseableArrayWrapper::operator=(ParseableArrayWrapper&& oth) {
        m_data = oth.m_data;
        m_size = oth.m_size;
        oth.m_data = nullptr;
        oth.m_size = 0;
    }

    char ParseableArrayWrapper::get(const size_t at) const {
        return at < m_size ? m_data[at] : '\0';
    }
    
    void ParseableArrayWrapper::read(char* ptr, const size_t len, const size_t at) const {
        if (at >= m_size) {
            memset(ptr, 0, len);
            return;
        }
        const size_t new_size = m_size - at;
        memcpy(ptr, m_data + at, len > new_size ? new_size : len);
        if (len > new_size) memset(ptr + new_size, 0, len - new_size);
    }
    
    size_t ParseableArrayWrapper::max_off() const {
        return m_size;
    }

} // namespace JSON
} // namespace Lunaris