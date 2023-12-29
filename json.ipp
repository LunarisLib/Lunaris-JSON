#pragma once

#include <limits.h>
#include <stdexcept>
#include "json.h"

namespace Lunaris {

    template<typename T, typename Tprec, typename std::enable_if<!std::is_floating_point<T>::value, int>::type>
    inline T strtoT_e(const char* s, const char** final_pos) {
        if (!s) return {};
        T gen{};
        // power -> engineer notation, power_dec -> from decimal only.
        Tprec power = 0, power_dec = 0;
        size_t p = 0;
        /*
        0: gen is the number itself
        1: engineer notation, positive power of 10
        2: engineer notation, negative power of 10
        3: decimal, may have engineer notation later, -= 1 for power each round
        */
        int8_t is_e = 0;
        bool is_neg = 0;

        while (s[p] != '\0' && is_e >= 0) {
            switch (s[p]) {
            case 'e':
            case 'E':
                is_e = 1; // assume positive for now
                power = 0; // forcing reset just to be sure...
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                switch (is_e) {
                case 0: // default number...
                    gen *= 10ULL;
                    gen += (static_cast<T>(s[p] - '0'));
                    break;
                case 1: // EXXX -> power of XXX
                    power *= 10;
                    power += ((int)(s[p] - '0'));
                    break;
                case 2: // E-XXX -> power of -XXX
                    power *= 10;
                    power -= ((int)(s[p] - '0'));
                    break;
                case 3: // still number part, but decimal
                    gen *= 10ULL;
                    gen += (static_cast<T>(s[p] - '0'));
                    power_dec -= 1;
                    break;
                default: break;
                }
                break;
            case '+':
                // still positive
                break;
            case '-':
                if (is_e == 1) is_e = 2; // was expecting engineer format, negative, ok
                else if (is_neg != 1 && p == 0) is_neg = 1; // was not negative, first case, go negative then
                else return 0; // There was another - somewhere where shouldn't be one
                break;
            case '.':
                if (is_e != 0) return 0; // was on engineer format, decimal power not supported. Drop.
                is_e = 3; // power -= 1
                break;
            default: // other chars not supported
                is_e = -1;
                break;
            }
            ++p;
        }

        //if (p > 0) --p; // back to pos
        if (final_pos) (*final_pos) = (s + p);

        power += power_dec; // combine both
        while (power > 0) { gen *= 10; --power; }
        while (power < 0) { gen /= 10; ++power; }

        if (is_neg == 1 && gen != 0) gen = ~(gen - 1);

        return gen;
    }

    template<typename T, typename Tprec, typename std::enable_if<std::is_floating_point<T>::value, int>::type>
    inline T strtoT_e(const char* s, const char** final_pos) {
        if (!s) return {};
        T gen{};
        // power -> engineer notation, power_dec -> from decimal only.
        Tprec power = 0, power_dec = 0;
        size_t p = 0;
        /*
        0: gen is the number itself
        1: engineer notation, positive power of 10
        2: engineer notation, negative power of 10
        3: decimal, may have engineer notation later, -= 1 for power each round
        */
        int8_t is_e = 0;
        bool is_neg = 0;

        while (s[p] != '\0' && is_e >= 0) {
            switch (s[p]) {
            case 'e':
            case 'E':
                is_e = 1; // assume positive for now
                power = 0; // forcing reset just to be sure...
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                switch (is_e) {
                case 0: // default number...
                    gen *= 10ULL;
                    gen += (static_cast<T>(s[p] - '0'));
                    break;
                case 1: // EXXX -> power of XXX
                    power *= 10;
                    power += ((int)(s[p] - '0'));
                    break;
                case 2: // E-XXX -> power of -XXX
                    power *= 10;
                    power -= ((int)(s[p] - '0'));
                    break;
                case 3: // still number part, but decimal
                    gen *= 10ULL;
                    gen += (static_cast<T>(s[p] - '0'));
                    power_dec -= 1;
                    break;
                default: break;
                }
                break;
            case '+':
                // still positive
                break;
            case '-':
                if (is_e == 1) is_e = 2; // was expecting engineer format, negative, ok
                else if (is_neg != 1 && p == 0) is_neg = 1; // was not negative, first case, go negative then
                else return 0; // There was another - somewhere where shouldn't be one
                break;
            case '.':
                if (is_e != 0) return 0; // was on engineer format, decimal power not supported. Drop.
                is_e = 3; // power -= 1
                break;
            default: // other chars not supported
                is_e = -1;
                break;
            }
            ++p;
        }

        //if (p > 0) --p; // back to pos
        if (final_pos) (*final_pos) = (s + p);

        power += power_dec; // combine both
        while (power > 0) { gen *= 10; --power; }
        while (power < 0) { gen /= 10; ++power; }

        if (is_neg == 1 && gen != 0) gen = -gen;

        return gen;
    }

    template<typename T, typename Tprec, typename std::enable_if<!std::is_floating_point<T>::value, int>::type>
    inline T hextoT(const char* s, const char** final_pos)
    {
        if (!s) return {};
        bool is_neg = s && s[0] == '-';
        if (is_neg || s[0] == '+') ++s;

        if (strncmp(s, "0x", 2) == 0 || strncmp(s, "0X", 2) == 0) s += 2; // offset 0x decl

        T gen{};
        size_t p = 0;
        bool get_out = false;

        while (s[p] != '\0' && !get_out) {

            switch (s[p]) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                gen <<= 4;
                gen += (static_cast<T>(s[p] - '0'));
                break;
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
                gen <<= 4;
                gen += (static_cast<T>(10 + s[p] - 'a'));
                break;
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
                gen <<= 4;
                gen += (static_cast<T>(10 + s[p] - 'A'));
                break;
            default:
                get_out = true;
                break;
            }

            ++p;
        }

        //if (p > 0) --p; // back to pos
        if (final_pos) (*final_pos) = (s + p);

        if (is_neg == 1 && gen != 0) gen = ~(gen - 1);

        return gen;
    }

    template<typename T, typename Tprec, typename std::enable_if<std::is_floating_point<T>::value, int>::type>
    inline T hextoT(const char* s, const char** final_pos)
    {
        if (!s) return {};
        bool is_neg = s && s[0] == '-';
        if (is_neg || s[0] == '+') ++s;

        if (strncmp(s, "0x", 2) == 0 || strncmp(s, "0X", 2) == 0) s += 2; // offset 0x decl

        int64_t gen{};
        size_t p = 0;
        bool get_out = false;

        while (s[p] != '\0' && !get_out) {

            switch (s[p]) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                gen <<= 4;
                gen += (static_cast<int64_t>(s[p] - '0'));
                break;
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
                gen <<= 4;
                gen += (static_cast<int64_t>(10 + s[p] - 'a'));
                break;
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
                gen <<= 4;
                gen += (static_cast<int64_t>(10 + s[p] - 'A'));
                break;
            default:
                get_out = true;
                break;
            }

            ++p;
        }

        //if (p > 0) --p; // back to pos
        if (final_pos) (*final_pos) = (s + p);

        if (is_neg == 1 && gen != 0) gen = ~(gen - 1);

        return static_cast<T>(gen);
    }

    template<typename T, typename Tprec>
    inline T autostrtoT(const char* s, const char** final_pos)
    {
        if ((s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) || (s[1] == '0' && (s[2] == 'x' || s[2] == 'X'))) return hextoT<T, Tprec>(s, final_pos);
        return strtoT_e<T, Tprec>(s, final_pos);
    }


    inline void __skip_next_spaces_auto(const char* buf, size_t& off)
    {
        while (buf[off] <= 32 && buf[off] != '\0') ++off;
    }

    inline void __skip_string_auto_escape(const char* buf, size_t& off)
    {
        while (buf[off] != '\"' && buf[off] != '\0') {
            if (buf[off] == '\\' && buf[off + 1] != '\0') ++off;
            ++off;
        }
    }
    inline void __skip_number(const char* buf, size_t& off)
    {
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



    inline JSON::ref* JSON::ref::make_child()
    {
        if (this->child) return this->child;
        return (this->child = new ref());
    }

    inline JSON::ref* JSON::ref::make_next()
    {
        if (this->next) return this->next;
        return (this->next = new ref());
    }

    inline void JSON::ref::free_next_and_child()
    {
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

    inline const char* JSON::nav::curr_off() const
    {
        return buf + off;
    }

    inline void JSON::nav::skip_next_spaces_auto()
    {
        __skip_next_spaces_auto(buf, off);
    }

    inline void JSON::nav::skip_string_auto_escape()
    {
        __skip_string_auto_escape(buf, off);
    }
    inline void JSON::nav::skip_number()
    {
        __skip_number(buf, off);
    }

    inline bool JSON::nav::eof() const
    {
        return buf[off] == '\0';
    }

    inline void JSON::prt::put(char ch)
    {
        if (fun) fun(ch);
        else target[target_off] = ch;
        ++target_off;
    }

    inline void JSON::_free()
    {
        if (!m_root) return;
        if (m_ref) {
            m_ref->free_next_and_child();
            delete m_ref;
            m_ref = nullptr;
        }
        if (m_charptr_clean) {
            delete[] m_charptr_clean;
            m_charptr_clean = nullptr;
        }
    }

    inline JSON::JSON(JSON::ref* r)
        : m_ref(r), m_root(false)
    {
    }

    inline void JSON::parse_value(ref* r, nav* n)
    {
        n->skip_next_spaces_auto();

        switch (n->curr_off()[0]) {
        case '-': case '+':
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            r->self_type = type::NUMBER;
            r->val_ptr = n->curr_off();
            n->skip_number();
            break;
        case '\"':
            r->self_type = type::STRING;
            ++n->off;
            r->val_ptr = n->curr_off();
            n->skip_string_auto_escape();
            ++n->off;
            break;
        case '{':
        {
            r->self_type = type::OBJECT;
            parse_object(r, n);
            n->skip_next_spaces_auto();
        }
            break;
        case '[':
        {
            r->self_type = type::ARRAY;
            //auto* new_obj = r->make_child();
            parse_array(r, n);
            n->skip_next_spaces_auto();
        }
            break;
        default:
            if (strncmp(n->curr_off(), "true", 4) == 0)
            {
                r->self_type = type::BOOL;
                r->val_ptr = n->curr_off();
                n->off += 4;
            }
            else if (strncmp(n->curr_off(), "false", 5) == 0)
            {
                r->self_type = type::BOOL;
                r->val_ptr = n->curr_off();
                n->off += 5;
            }
            else if (strncmp(n->curr_off(), "null", 4) == 0)
            {
                r->self_type = type::NIL;
                r->val_ptr = n->curr_off();
                n->off += 4;
            }
            break;
        }
    }

    inline void JSON::parse_object(ref* r, nav* n)
    {
        // should be at {
        ++n->off; // skip {
        n->skip_next_spaces_auto();

        if (n->curr_off()[0] == '}') { ++n->buf; return; } // empty object + skip }

        auto* nr = r->make_child();

        do {
            if (n->curr_off()[0] == ',') ++n->off;
            n->skip_next_spaces_auto();
            ++n->off; // skip "

            nr->key_ptr = n->curr_off();
            n->skip_string_auto_escape();
            ++n->off; // skip "
            n->skip_next_spaces_auto(); // skip until :
            if (*(n->buf + n->off) != ':') throw std::runtime_error("JSON malformed."); // "key": value... where is :?
            ++n->off; // skip :

            parse_value(nr, n);

            n->skip_next_spaces_auto();

            if (!n->eof() && n->curr_off()[0] != '}') {
                nr = nr->make_next();
            }
            else break;
        } while (1);
        ++n->off; // skip }
    }

    inline void JSON::parse_array(ref* r, nav* n)
    {
        // should be at {
        ++n->off; // skip {
        n->skip_next_spaces_auto();

        if (n->curr_off()[0] == ']') { ++n->buf; return; } // empty object + skip }

        auto* nr = r->make_child();

        do {
            if (n->curr_off()[0] == ',') ++n->off;
            n->skip_next_spaces_auto();

            nr->key_ptr = nullptr; // array item does not have key

            parse_value(nr, n);

            n->skip_next_spaces_auto();

            if (!n->eof() && n->curr_off()[0] != ']') {
                nr = nr->make_next();
            }
            else break;
        } while (1);
        ++n->off; // skip ]
    }

    inline size_t JSON::print_any(ref* ref, prt& f)
    {
        if (!ref) return 0;

        bool fnl = f.curr_depth == 0;

        const auto stronf = 
            [&f](const char* s) {
                while (*s != '\0') {
                    f.put(*s++);
                }
            };
        const auto strnonf = 
            [&f](const char* s, const char* e) {
                if (e < s) return;
                while (s != e) {
                    f.put(*s++);
                }
            };
        const auto spaceline =
            [&f, &fnl] {
                if (f.lining > 0 && !fnl) {
                    f.put('\n');

                    for (size_t k = 0; k < f.curr_depth * f.lining; ++k) {
                        f.put(f.space_char);
                    }
                } 
                fnl = false;
            };

        const auto print_key_if_exists = [&] {
            if (ref->key_ptr) {
                const char* key_beg = ref->key_ptr;
                const char* key_end = ref->key_ptr;
                size_t offsetter = 0;
                __skip_string_auto_escape(key_end, offsetter);
                key_end += offsetter;

                f.put('\"');
                strnonf(key_beg, key_end);
                stronf("\":");
            }
        };


        size_t offsetter = 0;
        while (1) {
            switch (ref->self_type) {
            case type::BOOL:
            {
                spaceline();
                print_key_if_exists();
                if (strncmp(ref->val_ptr, "true", 4) == 0)  stronf("true");
                else                                        stronf("false");
            }
            break;
            case type::NIL:
            {
                spaceline();
                print_key_if_exists();
                stronf("null");
            }
            break;
            case type::NUMBER:
            {
                spaceline();
                print_key_if_exists();

                const char* val_beg = ref->val_ptr;
                const char* val_end = ref->val_ptr;
                offsetter = 0;
                __skip_number(val_end, offsetter);
                val_end += offsetter;

                strnonf(val_beg, val_end);
            }
            break;
            case type::STRING:
            {
                spaceline();
                print_key_if_exists();

                const char* val_beg = ref->val_ptr;
                const char* val_end = ref->val_ptr;
                offsetter = 0;
                __skip_string_auto_escape(val_end, offsetter);
                val_end += offsetter;

                f.put('\"');
                strnonf(val_beg, val_end);
                f.put('\"');
            }
            break;
            case type::ARRAY:
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
            case type::OBJECT:
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

    inline JSON::JSON(const char* src, size_t len)
        : m_ref(new ref())
    {
        nav navigation{ src, 0 };
        parse_value(m_ref, &navigation);
    }
    
    inline JSON::JSON(JSON&& o)
        : m_ref(o.m_ref), m_root(o.m_root)
    {
        o.m_ref = nullptr;
    }

    inline JSON::~JSON()
    {
        _free();
    }
    inline JSON::type JSON::get_type() const
    {
        return m_ref ? m_ref->self_type : JSON::type::INVALID;
    }

    inline int64_t JSON::get_int() const
    {
        return m_ref ? autostrtoT<int64_t>(m_ref->val_ptr, nullptr) : int64_t{};
    }

    inline uint64_t JSON::get_uint() const
    {
        return m_ref ? autostrtoT<uint64_t>(m_ref->val_ptr, nullptr) : uint64_t{};
    }

    inline float JSON::get_float() const
    {
        return m_ref ? autostrtoT<float>(m_ref->val_ptr, nullptr) : float{};
    }

    inline double JSON::get_double() const
    {
        return m_ref ? autostrtoT<double>(m_ref->val_ptr, nullptr) : double{};
    }

    inline bool JSON::get_bool() const
    {
        return m_ref ? strncmp(m_ref->val_ptr, "true", 4) == 0 : false;
    }
    inline bool JSON::get_is_null() const
    {
        return m_ref ? strncmp(m_ref->val_ptr, "null", 4) == 0 : false;
    }

    template<typename T>
    inline T JSON::get_number() const
    {
        return m_ref ? autostrtoT<T>(m_ref->val_ptr, nullptr) : T{};
    }

    inline const char* JSON::get_string_noalloc() const
    {
        return m_ref ? m_ref->val_ptr : nullptr;
    }

    inline const char* JSON::get_string() const
    {
        if (!m_ref) return nullptr;

        const char* str_beg = m_ref->val_ptr;
        if (m_charptr_clean && strncmp(m_charptr_clean, str_beg, strlen(m_charptr_clean)) == 0) return m_charptr_clean;

        size_t len = 0;
        switch (m_ref->self_type) {
        case type::BOOL:
            return get_bool() ? "true" : "false";
        case type::NIL:
            return "null";
        case type::NUMBER:
            __skip_number(str_beg, len);
            break;
        case type::STRING:
            __skip_string_auto_escape(str_beg, len);
            break;
        default:
            return "";
        }

        if (m_charptr_clean) delete[] m_charptr_clean;
        m_charptr_clean = new char[len + 1];

        memcpy(m_charptr_clean, str_beg, len);
        m_charptr_clean[len] = '\0';

        return m_charptr_clean;
    }

    inline const char* JSON::get_key_noalloc() const
    {
        return m_ref ? m_ref->key_ptr : nullptr;
    }

    inline const char* JSON::get_key() const
    {
        if (!m_ref) return nullptr;

        const char* str_beg = m_ref->key_ptr;
        if (m_charptr_clean && strncmp(m_charptr_clean, str_beg, strlen(m_charptr_clean)) == 0) return m_charptr_clean;

        size_t len = 0;
        switch (m_ref->self_type) {
        case type::BOOL:
            return get_bool() ? "true" : "false";
        case type::NIL:
            return "null";
        case type::NUMBER:
            __skip_number(str_beg, len);
            break;
        case type::STRING:
            __skip_string_auto_escape(str_beg, len);
            break;
        default:
            return "";
        }

        if (m_charptr_clean) delete[] m_charptr_clean;
        m_charptr_clean = new char[len + 1];

        memcpy(m_charptr_clean, str_beg, len);
        m_charptr_clean[len] = '\0';

        return m_charptr_clean;
    }

    inline size_t JSON::print(printer_char_function f, const size_t lining, const char space_ch) const
    {
        if (!m_ref) return 0;
        prt p{ f, lining, 0, space_ch, nullptr, 0 };
        return print_any(m_ref, p);
    }
    inline size_t JSON::print_to(char* buf, const size_t lining, const char space_ch) const
    {
        if (!m_ref) return 0;
        prt p{ nullptr, lining, 0, space_ch, buf, 0 };
        return print_any(m_ref, p);
    }

    inline JSON JSON::operator[](const char* key) const
    {
        if (!m_ref || m_ref->self_type != type::OBJECT) return JSON((ref*)nullptr);

        for (ref* it = m_ref->child; it != nullptr; it = it->next)
        {
            if (it->key_ptr == nullptr && key == nullptr) return JSON(it);

            const char* key_beg = it->key_ptr;
            size_t keylen = 0;
            __skip_string_auto_escape(key_beg, keylen);
            if (it->key_ptr && strncmp(it->key_ptr, key, keylen) == 0) return JSON(it);
        }

        return JSON((ref*)nullptr);
    }

    inline JSON JSON::operator[](size_t idx) const
    {
        if (!m_ref || m_ref->self_type != type::ARRAY) return JSON((ref*)nullptr);

        for (ref* it = m_ref->child; it != nullptr; it = it->next)
        {
            if (idx-- == 0) return JSON(it);
        }

        return JSON((ref*)nullptr);
    }

    inline JSON JSON::operator[](int idx) const
    {
        if (idx < 0) return JSON((ref*)nullptr);
        return this->operator[](static_cast<size_t>(idx));
    }

    inline JSON::operator int16_t() const
    {
        return static_cast<int16_t>(this->get_int());
    }
    inline JSON::operator int32_t() const
    {
        return static_cast<int32_t>(this->get_int());
    }
    inline JSON::operator int64_t() const
    {
        return this->get_int();
    }
    inline JSON::operator uint16_t() const
    {
        return static_cast<uint16_t>(this->get_int());
    }
    inline JSON::operator uint32_t() const
    {
        return static_cast<uint32_t>(this->get_int());
    }
    inline JSON::operator uint64_t() const
    {
        return this->get_uint();
    }
    inline JSON::operator bool() const
    {
        return this->get_bool();
    }
    inline JSON::operator float() const
    {
        return this->get_float();
    }
    inline JSON::operator double() const
    {
        return this->get_double();
    }
    inline JSON::operator const char* () const
    {
        return this->get_string();
    }
}