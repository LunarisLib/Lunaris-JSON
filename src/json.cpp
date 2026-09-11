#include <cstring>

#include <Lunaris/JSON/exception.h>
#include <Lunaris/JSON/json.h>

namespace Lunaris {
namespace JSON {

    void __skip_next_spaces_auto(const char* buf, size_t& off) {
        while (buf[off] <= 32 && buf[off] != '\0') ++off;
    }

    void __skip_next_spaces_auto(const ParseableJson* const buf, size_t& off) {
        while (buf->get(off) <= 32 && buf->get(off) != '\0') ++off;
    }

    void __skip_string_auto_escape(const char* buf, size_t& off) {
        while (buf[off] != '\"' && buf[off] != '\0') {
            if (buf[off] == '\\' && buf[off + 1] != '\0') ++off;
            ++off;
        }
    }

    void __skip_string_auto_escape(const ParseableJson* const buf, size_t& off) {
        for (char boff = buf->get(off); boff != '\"' && boff != '\0'; boff = buf->get(off)) {
            if (boff == '\\' && boff != '\0') ++off;
            ++off;
        }
    }

    void __skip_number(const char* buf, size_t& off) {
        while (
            (
                buf[off] == '-' ||
                buf[off] == '+' ||
                buf[off] == '.' ||
                buf[off] == 'e' ||
                buf[off] == 'E' ||
                buf[off] == 'x' ||
                buf[off] == 'X' ||
                (buf[off] >= '0' && buf[off] <= '9') ||
                (buf[off] >= 'a' && buf[off] <= 'f') ||
                (buf[off] >= 'A' && buf[off] <= 'F')
                ) && buf[off] != '\0'
            ) ++off;
    }

    void __skip_number(const ParseableJson* const buf, size_t& off) {
        for (char boff = buf->get(off);
            (
                boff == '-' ||
                boff == '+' ||
                boff == '.' ||
                boff == 'e' ||
                boff == 'E' ||
                boff == 'x' ||
                boff == 'X' ||
                (boff >= '0' && boff <= '9') ||
                (boff >= 'a' && boff <= 'f') ||
                (boff >= 'A' && boff <= 'F')
                ) && boff != '\0';
        boff = buf->get(off)) ++off;
    }

    Json::Json(ParseableJson*&& readable_json) 
        : m_ref(new ref()), m_charptr_clean(nullptr), m_root(true), m_base(std::move(readable_json))
    {
        readable_json = nullptr;
        nav navigation{ m_base, 0, {} };
        parse_value(m_ref, &navigation);
    }

    Json::Json(Json&& oth)
        : m_ref(oth.m_ref), m_charptr_clean(std::move(oth.m_charptr_clean)), m_root(oth.m_root), m_base(oth.m_base) 
    {
        oth.m_ref = nullptr;
        oth.m_base = nullptr;
    }

    Json::~Json() {
        _free();
    }

    Json::e_type Json::get_type() const {
        return m_ref ? m_ref->self_type : e_type::INVALID;
    }

    int64_t Json::get_int() const {
        return m_ref ? autostrtoT<int64_t>(m_base, m_ref->get_val_ptr(m_base)) : int64_t{}; // safe on -1
    }

    uint64_t Json::get_uint() const {
        return m_ref ? autostrtoT<uint64_t>(m_base, m_ref->get_val_ptr(m_base)) : uint64_t{}; // safe on -1
    }

    float Json::get_float() const {
        return m_ref ? autostrtoT<float>(m_base, m_ref->get_val_ptr(m_base)) : float{}; // safe on -1
    }

    double Json::get_double() const {
        return m_ref ? autostrtoT<double>(m_base, m_ref->get_val_ptr(m_base)) : double{}; // safe on -1
    }

    bool Json::get_bool() const {
        return m_ref ? m_ref->is_eq_val_ptr_val(m_base, "true", 4) == 0 : false;
    }

    bool Json::get_is_null() const {
        return m_ref ? (m_ref->self_type == e_type::NIL) : false;
    }

    const char* Json::get_string() const {
        if (!m_ref) return nullptr;
        if (m_ref->self_type == e_type::NIL) return "null";

        if (m_charptr_clean &&
            m_ref->is_eq_val_ptr_val(m_base, m_charptr_clean.get(), strlen(m_charptr_clean.get())) == 0
        ) return m_charptr_clean.get();

        size_t str_beg = m_ref->get_val_ptr(m_base);

        if (str_beg == static_cast<size_t>(-1))
            throw JsonException("Unable to read position of value in base");

        size_t len = str_beg;
        switch (m_ref->self_type) {
        case e_type::BOOL:
            return get_bool() ? "true" : "false";
        case e_type::NIL:
            return "null";
        case e_type::NUMBER:
            __skip_number(m_base, len);
            break;
        case e_type::STRING:
            __skip_string_auto_escape(m_base, len);
            break;
        default:
            return "";
        }

        m_charptr_clean = std::make_unique<char[]>(len - str_beg + 1);            

        m_base->read(m_charptr_clean.get(), len - str_beg, str_beg);
        m_charptr_clean[len - str_beg] = '\0';

        return m_charptr_clean.get();
    }

    const char* Json::get_key() const {
        if (!m_ref) return nullptr;

        if (m_charptr_clean
            && m_ref->is_eq_key_ptr_val(m_base, m_charptr_clean.get(), strlen(m_charptr_clean.get())) == 0
        ) return m_charptr_clean.get();

        size_t str_beg = m_ref->key_ptr;
        if (str_beg == static_cast<size_t>(-1))
            throw JsonException("Unable to read position of value in base");

        size_t len = str_beg;
        switch (m_ref->self_type) {
        case e_type::BOOL:
            return get_bool() ? "true" : "false";
        case e_type::NIL:
            return "null";
        case e_type::NUMBER:
            __skip_number(m_base, len);
            break;
        case e_type::STRING:
            __skip_string_auto_escape(m_base, len);
            break;
        default:
            return "";
        }

        m_charptr_clean = std::make_unique<char[]>(len - str_beg + 1);

        m_base->read(m_charptr_clean.get(), len - str_beg, str_beg);
        m_charptr_clean[len - str_beg] = '\0';

        return m_charptr_clean.get();
    }

    size_t Json::print(printer_char_function f, const size_t lining, const char space_ch) const {
        if (!m_ref) return 0;
        prt p{ m_base, f, lining, 0, space_ch, nullptr, 0 };
        return print_any(m_ref, p);
    }

    size_t Json::print_to(char* buf, const size_t lining, const char space_ch) const {
        if (!m_ref) return 0;
        prt p{ m_base, nullptr, lining, 0, space_ch, buf, 0 };
        return print_any(m_ref, p);
    }

    Json Json::operator[](const char* key) const {
        if (!m_ref || m_ref->self_type != e_type::OBJECT) return Json(nullptr, nullptr);

        for (ref* it = m_ref->child; it != nullptr; it = it->next)
        {
            if (it->is_eq_key_ptr_val(m_base, key, strlen(key)))
                return Json(it, m_base);
        }

        return Json(nullptr, nullptr);
    }

    Json Json::operator[](size_t idx) const {
        if (!m_ref || m_ref->self_type != e_type::ARRAY) return Json(nullptr, nullptr);

        for (ref* it = m_ref->child; it != nullptr; it = it->next)
        {
            if (idx-- == 0) return Json(it, m_base);
        }

        return Json(nullptr, nullptr);
    }

    Json Json::operator[](int idx) const {
        if (idx < 0) return Json(nullptr, nullptr);
        return this->operator[](static_cast<size_t>(idx));
    }

    Json::operator int16_t() const {
        return static_cast<int16_t>(this->get_int());
    }

    Json::operator int32_t() const {
        return static_cast<int32_t>(this->get_int());
    }

    Json::operator int64_t() const {
        return this->get_int();
    }

    Json::operator uint16_t() const {
        return static_cast<uint16_t>(this->get_uint());
    }

    Json::operator uint32_t() const {
        return static_cast<uint32_t>(this->get_uint());
    }

    Json::operator uint64_t() const {
        return this->get_uint();
    }

    Json::operator bool() const {
        return this->get_bool();
    }

    Json::operator float() const {
        return this->get_float();
    }

    Json::operator double() const {
        return this->get_double();
    }

    Json::operator const char*() const {
        return this->get_string();
    }
    

    Json::ref* Json::ref::make_child() {
        if (this->child) return this->child;
        return (this->child = new ref());
    }

    Json::ref* Json::ref::make_next() {
        if (this->next) return this->next;
        return (this->next = new ref());
    }

    void Json::ref::free_next_and_child() {
        if (this->next) {
            this->next->free_next_and_child();
            delete this->next;
            this->next = nullptr;
        }
        if (this->child) {
            this->child->free_next_and_child();
            delete this->child;
            this->child = nullptr;
        }
    }

    size_t Json::ref::get_val_ptr(const ParseableJson* const base) const {
        if (key_is_val) return key_ptr; // array objects don't have a key, so the key will be used as the value
        if (self_type == e_type::NIL) return static_cast<size_t>(-1); // quick

        if (!key_ptr) return static_cast<size_t>(-1);
        //size_t ref = key_ptr;
        size_t off = key_ptr;

        // Skip key
        __skip_string_auto_escape(base, off); // its string val
        ++off; // must be \"
        __skip_next_spaces_auto(base, off); // advance to :
        if (base->get(off) != ':') return static_cast<size_t>(-1); // then :
        ++off; // skip :
        __skip_next_spaces_auto(base, off); // advance to val


        if (self_type == e_type::STRING) {
            if (base->get(off) != '\"') return static_cast<size_t>(-1); // should start work "
            ++off;
        }

        return off;
    }

    bool Json::ref::is_eq_key_ptr_val(const ParseableJson* const base, const char* str, const size_t l) const {
        if (l == 0) return true;
        if (key_ptr == static_cast<size_t>(-1)) return false;

        size_t keylen = key_ptr;
        __skip_string_auto_escape(base, keylen);

        if (keylen - key_ptr != l) return false; // size of key is less than size of test, so test has more chars.

        char* tmp = new char[l];
        base->read(tmp, l, key_ptr);
        const bool eq = memcmp(tmp, str, l) == 0;
        delete[] tmp;

        return eq;
    }

    bool Json::ref::is_eq_val_ptr_val(const ParseableJson* const base, const char* str, const size_t l) const {
        if (l == 0) return true;
        const size_t at = get_val_ptr(base);
        if (at == static_cast<size_t>(-1)) return false;

        char* tmp = new char[l];
        base->read(tmp, l, at);
        const bool eq = memcmp(tmp, str, l) == 0;
        delete[] tmp;

        return eq;
    }

    char Json::nav::curr_ch() const {
        return base->get(off);
    }

    const char* Json::nav::curr_off() const {
        memset(minibuf, '\0', sizeof(minibuf));
        base->read(minibuf, 7, off); // max 7 + '\0'
        return minibuf;
    }

    void Json::nav::skip_next_spaces_auto() {
        __skip_next_spaces_auto(base, off);
    }

    void Json::nav::skip_string_auto_escape() {
        __skip_string_auto_escape(base, off);
    }

    void Json::nav::skip_number() {
        __skip_number(base, off);
    }

    bool Json::nav::eof() const {
        return base->get(off) == '\0';
    }

    void Json::prt::put(char ch) {
        if (fun) fun(ch);
        else if (target) target[target_off] = ch;
        ++target_off;
    }


    void Json::_free() {
        if (!m_root) return;
        if (m_ref) {
            m_ref->free_next_and_child();
            delete m_ref;
            m_ref = nullptr;
        }
        m_charptr_clean.reset();
        if (m_base) {
            delete m_base;
            m_base = nullptr;
        }
    }

    Json::Json(Json::ref* r, ParseableJson* jr)
        : m_ref(r), m_charptr_clean(nullptr), m_root(false), m_base(jr)
    {}

    char Json::get_val_of(ref* r) {
        return r ? m_base->get(r->key_ptr) : '\0';
    }

    void Json::parse_value(ref* r, nav* n) {
        n->skip_next_spaces_auto();

        switch (n->curr_ch()) {
        case '-': case '+':
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            r->self_type = e_type::NUMBER;
            if (r->key_is_val) r->key_ptr = n->off;// n->curr_off();
            n->skip_number();
            break;
        case '\"':
            r->self_type = e_type::STRING;
            ++n->off;
            if (r->key_is_val) r->key_ptr = n->off;// n->curr_off();
            n->skip_string_auto_escape();
            ++n->off;
            break;
        case '{':
        {
            r->self_type = e_type::OBJECT;
            parse_object(r, n);
            n->skip_next_spaces_auto();
        }
            break;
        case '[':
        {
            r->self_type = e_type::ARRAY;
            parse_array(r, n);
            n->skip_next_spaces_auto();
        }
            break;
        default:
            const char* curr = n->curr_off();

            if (strncmp(curr, "true", 4) == 0)
            {
                r->self_type = e_type::BOOL;
                if (r->key_is_val) r->key_ptr = n->off;//  n->curr_off();
                n->off += 4;
            }
            else if (strncmp(curr, "false", 5) == 0)
            {
                r->self_type = e_type::BOOL;
                if (r->key_is_val) r->key_ptr = n->off;//  n->curr_off();
                n->off += 5;
            }
            else if (strncmp(curr, "null", 4) == 0)
            {
                r->self_type = e_type::NIL;
                if (r->key_is_val) r->key_ptr = n->off;//  n->curr_off();
                n->off += 4;
            }
            break;
        }
    }

    void Json::parse_object(ref* r, nav* n) {
        // should be at {
        ++n->off; // skip {
        n->skip_next_spaces_auto();

        if (n->curr_ch() == '}') { ++n->off; return; } // empty object + skip }

        auto* nr = r->make_child();

        do {
            if (n->curr_ch() == ',') ++n->off;
            n->skip_next_spaces_auto();
            ++n->off; // skip "

            nr->key_ptr = n->off;
            n->skip_string_auto_escape();
            ++n->off; // skip "
            n->skip_next_spaces_auto(); // skip until :
            if (n->curr_ch() != ':') throw JsonException("JSON malformed."); // "key": value... where is :?
            ++n->off; // skip :

            parse_value(nr, n);

            n->skip_next_spaces_auto();

            if (!n->eof() && n->curr_ch() != '}') {
                nr = nr->make_next();
            }
            else break;
        } while (1);
        ++n->off; // skip }
    }

    void Json::parse_array(ref* r, nav* n) {
        // should be at {
        ++n->off; // skip {
        n->skip_next_spaces_auto();

        if (n->curr_ch() == ']') { ++n->off; return; } // empty object + skip }

        auto* nr = r->make_child();

        do {
            if (n->curr_ch() == ',') ++n->off;
            n->skip_next_spaces_auto();

            nr->key_ptr = static_cast<size_t>(-1); // array item does not have key
            nr->key_is_val = true;

            parse_value(nr, n);

            n->skip_next_spaces_auto();

            if (!n->eof() && n->curr_ch() != ']') {
                nr = nr->make_next();
            }
            else break;
        } while (1);
        ++n->off; // skip ]
    }

    size_t Json::print_any(ref* ref, prt& f) {
        if (!ref) return 0;

        bool fnl = f.curr_depth == 0;

        // print until \0
        const auto stronf = [&f](const char* s) { while (*s != '\0') { f.put(*s++); } };
        // print from point s (start) to e (end)
        const auto strnonf = [&f](size_t s, const size_t e) { if (e < s) return; while (s != e) { f.put(f.m_base->get(s++)); } };
        // automatic breakline when necessary
        const auto spaceline = [&f, &fnl] { 
            if (f.lining > 0 && !fnl) { 
                f.put('\n');
                for (size_t k = 0; k < f.curr_depth * f.lining; ++k) { f.put(f.space_char); }
            }
            fnl = false;
        };
        // if type has key and key is set, print key
        const auto print_key_if_exists = [&] {
            if (!ref->key_is_val && ref->key_ptr) {
                size_t key_beg = ref->key_ptr;
                size_t key_end = ref->key_ptr;
                __skip_string_auto_escape(f.m_base, key_end);
                f.put('\"'); strnonf(key_beg, key_end); stronf("\":");
            }
        };

        while (1) {
            switch (ref->self_type) {
            case e_type::BOOL:
            {
                spaceline();
                print_key_if_exists();
                if (ref->is_eq_val_ptr_val(f.m_base, "true", 4) == 0)  stronf("true");
                else                                                   stronf("false");
            }
            break;
            case e_type::NIL:
            {
                spaceline();
                print_key_if_exists();
                stronf("null");
            }
            break;
            case e_type::NUMBER:
            {
                spaceline();
                print_key_if_exists();

                size_t val_beg = ref->get_val_ptr(f.m_base);
                if (val_beg != static_cast<size_t>(-1)) {
                    size_t val_end = val_beg;
                    __skip_number(f.m_base, val_end);

                    strnonf(val_beg, val_end);
                }
                else {
                    throw JsonException("Invalid position read at print_any NUMBER case");
                }
            }
            break;
            case e_type::STRING:
            {
                spaceline();
                print_key_if_exists();

                size_t val_beg = ref->get_val_ptr(f.m_base);
                if (val_beg != static_cast<size_t>(-1)) {
                    size_t val_end = val_beg;
                    __skip_string_auto_escape(f.m_base, val_end);

                    f.put('\"');
                    strnonf(val_beg, val_end);
                    f.put('\"');
                }
                else {
                    throw JsonException("Invalid position read at print_any STRING case");
                }
            }
            break;
            case e_type::ARRAY:
            {
                spaceline();
                print_key_if_exists();
                f.put('[');
                ++f.curr_depth;
                print_any(ref->child, f);
                --f.curr_depth;
                spaceline();
                f.put(']');
            }
            break;
            case e_type::OBJECT:
            {
                spaceline();
                print_key_if_exists();
                f.put('{');
                ++f.curr_depth;
                print_any(ref->child, f);
                --f.curr_depth;
                spaceline();
                f.put('}');
            }
            break;
            default: // nothing, but some compilers don't like missing stuff.
            break;
            }

            if (ref->next) {
                f.put(',');
                ref = ref->next;
            }
            else break;
        }

        if (f.curr_depth == 0) {
            f.put('\n');
        }

        return f.target_off - 1; // last \n
    }


} // namespace JSON
} // namespace Lunaris