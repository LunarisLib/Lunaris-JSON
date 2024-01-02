#pragma once

#include <limits.h>
#include <stdexcept>
#include "json.h"

namespace Lunaris {

    template<typename T, typename Tprec, typename std::enable_if<!std::is_floating_point<T>::value, int>::type>
    inline T strtoT_e(const IterateableJSONRef* const src, const size_t boff) {
        if (!src || boff == static_cast<size_t>(-1)) return {};
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

        for (char sp = src->get(boff + p); sp != '\0' && is_e >= 0; sp = src->get(boff + p)) {
            switch (sp) {
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
                    gen += (static_cast<T>(sp - '0'));
                    break;
                case 1: // EXXX -> power of XXX
                    power *= 10;
                    power += ((int)(sp - '0'));
                    break;
                case 2: // E-XXX -> power of -XXX
                    power *= 10;
                    power -= ((int)(sp - '0'));
                    break;
                case 3: // still number part, but decimal
                    gen *= 10ULL;
                    gen += (static_cast<T>(sp - '0'));
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
        //if (final_pos) (*final_pos) = (s + p);

        power += power_dec; // combine both
        while (power > 0) { gen *= 10; --power; }
        while (power < 0) { gen /= 10; ++power; }

        if (is_neg == 1 && gen != 0) gen = ~(gen - 1);

        return gen;
    }

    template<typename T, typename Tprec, typename std::enable_if<std::is_floating_point<T>::value, int>::type>
    inline T strtoT_e(const IterateableJSONRef* const src, const size_t boff) {
        if (!src || boff == static_cast<size_t>(-1)) return {};
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

        for (char sp = src->get(boff + p); sp != '\0' && is_e >= 0; sp = src->get(boff + p)) {
            switch (sp) {
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
                    gen += (static_cast<T>(sp - '0'));
                    break;
                case 1: // EXXX -> power of XXX
                    power *= 10;
                    power += ((int)(sp - '0'));
                    break;
                case 2: // E-XXX -> power of -XXX
                    power *= 10;
                    power -= ((int)(sp - '0'));
                    break;
                case 3: // still number part, but decimal
                    gen *= 10ULL;
                    gen += (static_cast<T>(sp - '0'));
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
        //if (final_pos) (*final_pos) = (s + p);

        power += power_dec; // combine both
        while (power > 0) { gen *= 10; --power; }
        while (power < 0) { gen /= 10; ++power; }

        if (is_neg == 1 && gen != 0) gen = -gen;

        return gen;
    }

    template<typename T, typename Tprec, typename std::enable_if<!std::is_floating_point<T>::value, int>::type>
    inline T hextoT(const IterateableJSONRef* const src, size_t boff)
    {
        if (!src || boff == static_cast<size_t>(-1)) return {};
        char sb[2]{};
        src->read(sb, 2, boff);

        bool is_neg = sb[0] == '-';
        if (is_neg || sb[0] == '+') ++boff;

        if (strncmp(sb, "0x", 2) == 0 || strncmp(sb, "0X", 2) == 0) boff += 2; // offset 0x decl

        T gen{};
        size_t p = 0;
        bool get_out = false;

        for (char sp = src->get(boff + p); sp != '\0' && !get_out; sp = src->get(boff + p)) {
            switch (sp) {
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
                gen += (static_cast<T>(sp - '0'));
                break;
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
                gen <<= 4;
                gen += (static_cast<T>(10 + sp - 'a'));
                break;
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
                gen <<= 4;
                gen += (static_cast<T>(10 + sp - 'A'));
                break;
            default:
                get_out = true;
                break;
            }

            ++p;
        }

        //if (p > 0) --p; // back to pos
        //if (final_pos) (*final_pos) = (s + p);

        if (is_neg == 1 && gen != 0) gen = ~(gen - 1);

        return gen;
    }

    template<typename T, typename Tprec, typename std::enable_if<std::is_floating_point<T>::value, int>::type>
    inline T hextoT(const IterateableJSONRef* const src, size_t boff)
    {
        if (!src || boff == static_cast<size_t>(-1)) return {};
        char sb[2]{};
        src->read(sb, 2, boff);

        bool is_neg = sb[0] == '-';
        if (is_neg || sb[0] == '+') ++boff;

        if (strncmp(sb, "0x", 2) == 0 || strncmp(sb, "0X", 2) == 0) boff += 2; // offset 0x decl

        int64_t gen{};
        size_t p = 0;
        bool get_out = false;
        
        for (char sp = src->get(boff + p); sp != '\0' && !get_out; sp = src->get(boff + p)) {
            switch (sp) {
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
                gen += (static_cast<int64_t>(sp - '0'));
                break;
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
                gen <<= 4;
                gen += (static_cast<int64_t>(10 + sp - 'a'));
                break;
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
                gen <<= 4;
                gen += (static_cast<int64_t>(10 + sp - 'A'));
                break;
            default:
                get_out = true;
                break;
            }

            ++p;
        }

        //if (p > 0) --p; // back to pos
        //if (final_pos) (*final_pos) = (s + p);

        if (is_neg == 1 && gen != 0) gen = ~(gen - 1);

        return static_cast<T>(gen);
    }

    template<typename T, typename Tprec>
    inline T autostrtoT(const IterateableJSONRef* const src, const size_t boff)
    {
        const char s0 = src->get(boff);
        const char s1 = src->get(boff + 1);
        const char s2 = src->get(boff + 2);
        if ((s0 == '0' && (s1 == 'x' || s1 == 'X')) || (s1 == '0' && (s2 == 'x' || s2 == 'X'))) return hextoT<T, Tprec>(src, boff);
        return strtoT_e<T, Tprec>(src, boff);
    }


    inline void __skip_next_spaces_auto(const char* buf, size_t& off)
    {
        while (buf[off] <= 32 && buf[off] != '\0') ++off;
    }
    inline void __skip_next_spaces_auto(const IterateableJSONRef* const buf, size_t& off)
    {
        while (buf->get(off) <= 32 && buf->get(off) != '\0') ++off;
    }

    inline void __skip_string_auto_escape(const char* buf, size_t& off)
    {
        while (buf[off] != '\"' && buf[off] != '\0') {
            if (buf[off] == '\\' && buf[off + 1] != '\0') ++off;
            ++off;
        }
    }
    inline void __skip_string_auto_escape(const IterateableJSONRef* const buf, size_t& off)
    {
        for (char boff = buf->get(off); boff != '\"' && boff != '\0'; boff = buf->get(off)) {
            if (boff == '\\' && boff != '\0') ++off;
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
    inline void __skip_number(const IterateableJSONRef* const buf, size_t& off)
    {
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

    inline size_t JSON::ref::get_val_ptr(const IterateableJSONRef* const base) const
    {
        if (key_is_val) return key_ptr; // array objects don't have a key, so the key will be used as the value
        if (self_type == type::NIL) return static_cast<size_t>(-1); // quick

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


        if (self_type == type::STRING) {
            if (base->get(off) != '\"') return static_cast<size_t>(-1); // should start work "
            ++off;
        }

        return off;
    }

    inline bool Lunaris::JSON::ref::is_eq_key_ptr_val(const IterateableJSONRef* const base, const char* str, const size_t l) const
    {
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

    inline bool Lunaris::JSON::ref::is_eq_val_ptr_val(const IterateableJSONRef* const base, const char* str, const size_t l) const
    {
        if (l == 0) return true;
        const size_t at = get_val_ptr(base);
        if (at == static_cast<size_t>(-1)) return false;

        char* tmp = new char[l];
        base->read(tmp, l, at);
        const bool eq = memcmp(tmp, str, l) == 0;
        delete[] tmp;

        return eq;
    }

    inline char JSON::nav::curr_ch() const
    {
        return base->get(off);
    }

    inline const char* JSON::nav::curr_off() const
    {
        memset(minibuf, '\0', sizeof(minibuf));
        base->read(minibuf, 7, off); // max 7 + '\0'
        return minibuf;
    }

    inline void JSON::nav::skip_next_spaces_auto()
    {
        __skip_next_spaces_auto(base, off);
    }

    inline void JSON::nav::skip_string_auto_escape()
    {
        __skip_string_auto_escape(base, off);
    }
    inline void JSON::nav::skip_number()
    {
        __skip_number(base, off);
    }

    inline bool JSON::nav::eof() const
    {
        return base->get(off) == '\0';
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
        if (m_base) {
            delete m_base;
            m_base = nullptr;
        }
    }

    inline JSON::JSON(JSON::ref* r, IterateableJSONRef* jr)
        : m_ref(r), m_charptr_clean(nullptr), m_root(false), m_base(jr)
    {
    }

    inline char JSON::get_val_of(ref* r)
    {
        return r ? m_base->get(r->key_ptr) : '\0';
    }

    inline void JSON::parse_value(ref* r, nav* n)
    {
        n->skip_next_spaces_auto();


        switch (n->curr_ch()) {
        case '-': case '+':
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            r->self_type = type::NUMBER;
            if (r->key_is_val) r->key_ptr = n->off;// n->curr_off();
            n->skip_number();
            break;
        case '\"':
            r->self_type = type::STRING;
            ++n->off;
            if (r->key_is_val) r->key_ptr = n->off;// n->curr_off();
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
            parse_array(r, n);
            n->skip_next_spaces_auto();
        }
            break;
        default:
            const char* curr = n->curr_off();

            if (strncmp(curr, "true", 4) == 0)
            {
                r->self_type = type::BOOL;
                if (r->key_is_val) r->key_ptr = n->off;//  n->curr_off();
                n->off += 4;
            }
            else if (strncmp(curr, "false", 5) == 0)
            {
                r->self_type = type::BOOL;
                if (r->key_is_val) r->key_ptr = n->off;//  n->curr_off();
                n->off += 5;
            }
            else if (strncmp(curr, "null", 4) == 0)
            {
                r->self_type = type::NIL;
                if (r->key_is_val) r->key_ptr = n->off;//  n->curr_off();
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
            if (n->curr_ch() != ':') throw std::runtime_error("JSON malformed."); // "key": value... where is :?
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

    inline void JSON::parse_array(ref* r, nav* n)
    {
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
            [&f](size_t s, const size_t e) {
                if (e < s) return;
                while (s != e) {
                    f.put(f.m_base->get(s++));
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
                size_t key_beg = ref->key_ptr;
                size_t key_end = ref->key_ptr;
                __skip_string_auto_escape(f.m_base, key_end);

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
                if (ref->is_eq_val_ptr_val(f.m_base, "true", 4) == 0)  stronf("true");
                else                                                   stronf("false");
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

                size_t val_beg = ref->get_val_ptr(f.m_base);
                if (val_beg != static_cast<size_t>(-1)) {
                    size_t val_end = val_beg;
                    __skip_number(f.m_base, val_end);

                    strnonf(val_beg, val_end);
                }
                else {
                    throw std::runtime_error("Invalid position read at print_any NUMBER case");
                }
            }
            break;
            case type::STRING:
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
                    throw std::runtime_error("Invalid position read at print_any STRING case");
                }
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
        : m_ref(new ref()), m_charptr_clean(nullptr), m_root(true), m_base(new CharPtrReferencer(src, len))
    {
        nav navigation{ m_base, 0, {} };
        parse_value(m_ref, &navigation);
    }

    inline JSON::JSON(IterateableJSONRef*&& src)
        : m_ref(new ref()), m_charptr_clean(nullptr), m_root(true), m_base(src)
    {
        src = nullptr;
        nav navigation{ m_base, 0, {} };
        parse_value(m_ref, &navigation);
    }
    
    inline JSON::JSON(JSON&& o) noexcept
        : m_ref(o.m_ref), m_charptr_clean(o.m_charptr_clean), m_root(o.m_root), m_base(o.m_base)
    {
        o.m_ref = nullptr;
        o.m_charptr_clean = nullptr;
        o.m_base = nullptr;
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
        return m_ref ? autostrtoT<int64_t>(m_base, m_ref->get_val_ptr(m_base)) : int64_t{}; // safe on -1
    }

    inline uint64_t JSON::get_uint() const
    {
        return m_ref ? autostrtoT<uint64_t>(m_base, m_ref->get_val_ptr(m_base)) : uint64_t{}; // safe on -1
    }

    inline float JSON::get_float() const
    {
        return m_ref ? autostrtoT<float>(m_base, m_ref->get_val_ptr(m_base)) : float{}; // safe on -1
    }

    inline double JSON::get_double() const
    {
        return m_ref ? autostrtoT<double>(m_base, m_ref->get_val_ptr(m_base)) : double{}; // safe on -1
    }

    inline bool JSON::get_bool() const
    {
        return m_ref ? m_ref->is_eq_val_ptr_val(m_base, "true", 4) == 0 : false;
    }
    inline bool JSON::get_is_null() const
    {
        return m_ref ? (m_ref->self_type == type::NIL) : false;
    }

    template<typename T>
    inline T JSON::get_number() const
    {
        return m_ref ? autostrtoT<T>(m_base, m_ref->get_val_ptr(m_base)) : T{}; // safe on -1
    }

    //inline const char* JSON::get_string() const
    //{
    //    if (!m_ref) return nullptr;
    //
    //    const char* str_beg = m_ref->get_val_ptr();
    //    if (m_charptr_clean && strncmp(m_charptr_clean, str_beg, strlen(m_charptr_clean)) == 0) return m_charptr_clean;
    //
    //    size_t len = 0;
    //    switch (m_ref->self_type) {
    //    case type::BOOL:
    //        return get_bool() ? "true" : "false";
    //    case type::NIL:
    //        return "null";
    //    case type::NUMBER:
    //        __skip_number(str_beg, len);
    //        break;
    //    case type::STRING:
    //        __skip_string_auto_escape(str_beg, len);
    //        break;
    //    default:
    //        return "";
    //    }
    //
    //    if (m_charptr_clean) delete[] m_charptr_clean;
    //    m_charptr_clean = new char[len + 1];
    //
    //    memcpy(m_charptr_clean, str_beg, len);
    //    m_charptr_clean[len] = '\0';
    //
    //    return m_charptr_clean;
    //}

    inline const char* JSON::get_string() const
    {
        if (!m_ref) return nullptr;

        if (m_charptr_clean && m_ref->is_eq_val_ptr_val(m_base, m_charptr_clean, strlen(m_charptr_clean)) == 0) return m_charptr_clean;
        size_t str_beg = m_ref->get_val_ptr(m_base);
        if (str_beg == static_cast<size_t>(-1)) throw std::runtime_error("Unable to read position of value in base");

        size_t len = str_beg;
        switch (m_ref->self_type) {
        case type::BOOL:
            return get_bool() ? "true" : "false";
        case type::NIL:
            return "null";
        case type::NUMBER:
            __skip_number(m_base, len);
            break;
        case type::STRING:
            __skip_string_auto_escape(m_base, len);
            break;
        default:
            return "";
        }

        if (m_charptr_clean) delete[] m_charptr_clean;
        m_charptr_clean = new char[len - str_beg + 1] {};

        m_base->read(m_charptr_clean, len - str_beg, str_beg);
        m_charptr_clean[len - str_beg] = '\0';

        return m_charptr_clean;
    }

    inline const char* JSON::get_key() const
    {
        if (!m_ref) return nullptr;

        if (m_charptr_clean && m_ref->is_eq_key_ptr_val(m_base, m_charptr_clean, strlen(m_charptr_clean)) == 0) return m_charptr_clean;

        size_t str_beg = m_ref->key_ptr;
        if (str_beg == static_cast<size_t>(-1)) throw std::runtime_error("Unable to read position of value in base");

        size_t len = str_beg;
        switch (m_ref->self_type) {
        case type::BOOL:
            return get_bool() ? "true" : "false";
        case type::NIL:
            return "null";
        case type::NUMBER:
            __skip_number(m_base, len);
            break;
        case type::STRING:
            __skip_string_auto_escape(m_base, len);
            break;
        default:
            return "";
        }

        if (m_charptr_clean) delete[] m_charptr_clean;
        m_charptr_clean = new char[len - str_beg + 1] {};

        m_base->read(m_charptr_clean, len - str_beg, str_beg);
        m_charptr_clean[len - str_beg] = '\0';

        return m_charptr_clean;
    }

    inline size_t JSON::print(printer_char_function f, const size_t lining, const char space_ch) const
    {
        if (!m_ref) return 0;
        prt p{ m_base, f, lining, 0, space_ch, nullptr, 0 };
        return print_any(m_ref, p);
    }
    inline size_t JSON::print_to(char* buf, const size_t lining, const char space_ch) const
    {
        if (!m_ref) return 0;
        prt p{ m_base, nullptr, lining, 0, space_ch, buf, 0 };
        return print_any(m_ref, p);
    }

    inline JSON JSON::operator[](const char* key) const
    {
        if (!m_ref || m_ref->self_type != type::OBJECT) return JSON(nullptr, nullptr);

        for (ref* it = m_ref->child; it != nullptr; it = it->next)
        {
            if (it->is_eq_key_ptr_val(m_base, key, strlen(key))) return JSON(it, m_base);
        }

        return JSON(nullptr, nullptr);
    }

    inline JSON JSON::operator[](size_t idx) const
    {
        if (!m_ref || m_ref->self_type != type::ARRAY) return JSON(nullptr, nullptr);

        for (ref* it = m_ref->child; it != nullptr; it = it->next)
        {
            if (idx-- == 0) return JSON(it, m_base);
        }

        return JSON(nullptr, nullptr);
    }

    inline JSON JSON::operator[](int idx) const
    {
        if (idx < 0) return JSON(nullptr, nullptr);
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



    inline CharPtrReferencer::CharPtrReferencer(const char* ref, const size_t len)
        : IterateableJSONRef(), m_ref(ref), m_max(len)
    {
    }

    inline char CharPtrReferencer::get(const size_t at) const
    {
        return at >= m_max ? '\0' : m_ref[at];
    }
    inline void CharPtrReferencer::read(char* ptr, const size_t len, const size_t at) const
    {
        if (at > m_max) return;
        memcpy(ptr, m_ref + at, len + at > m_max ? m_max - at : len);
    }
    inline size_t CharPtrReferencer::max_off() const
    {
        return m_max;
    }
    inline const char* CharPtrReferencer::raw() const
    {
        return m_ref;
    }
}