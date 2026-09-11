#include <cstring>

namespace Lunaris {
namespace JSON {

    using Tprec = int32_t;

    template<typename T>
    inline T Json::get_number() const
    {
        return m_ref ? autostrtoT<T>(m_base, m_ref->get_val_ptr(m_base)) : T{}; // safe on -1
    }

    template<typename T, typename std::enable_if<!std::is_floating_point<T>::value, int>::type>
    inline T Json::strtoT_e(const std::shared_ptr<ParseableJson>& src, const size_t boff) const {
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
        uint8_t is_e = 0;
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

        power += power_dec; // combine both
        while (power > 0) { gen *= 10; --power; }
        while (power < 0) { gen /= 10; ++power; }

        if (is_neg == 1 && gen != 0) gen = ~(gen - 1);

        return gen;
    }

    template<typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type>
    inline T Json::strtoT_e(const std::shared_ptr<ParseableJson>& src, const size_t boff) const {
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
        uint8_t is_e = 0;
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

        power += power_dec; // combine both
        while (power > 0) { gen *= 10; --power; }
        while (power < 0) { gen /= 10; ++power; }

        if (is_neg == 1 && gen != 0) gen = -gen;

        return gen;
    }

    template<typename T, typename std::enable_if<!std::is_floating_point<T>::value, int>::type>
    inline T Json::hextoT(const std::shared_ptr<ParseableJson>& src, size_t boff) const {
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

        if (is_neg == 1 && gen != 0) gen = ~(gen - 1);

        return gen;
    }

    template<typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type>
    inline T Json::hextoT(const std::shared_ptr<ParseableJson>& src, size_t boff) const {
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

    template<typename T>
    inline T Json::autostrtoT(const std::shared_ptr<ParseableJson>& src, const size_t boff) const {
        const char s0 = src->get(boff);
        const char s1 = src->get(boff + 1);
        const char s2 = src->get(boff + 2);
        if ((s0 == '0' && (s1 == 'x' || s1 == 'X')) || (s1 == '0' && (s2 == 'x' || s2 == 'X'))) return hextoT<T>(src, boff);
        return strtoT_e<T>(src, boff);
    }

} // namespace JSON
} // namespace Lunaris