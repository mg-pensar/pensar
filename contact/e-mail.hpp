// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef EMAIL_HPP
#define EMAIL_HPP


#include <string>

#include "contact.hpp"
#include "../cpp/src/s.hpp"
#include "../cpp/src/error.hpp"


namespace pensar_digital
{
    namespace cpplib
    {
        namespace contact
        {
            /// \brief Validates the local part of an email address according to RFC 5322. The local part is the part before the @ symbol.
            /// Simplified: allows alphanumeric, dot, underscore, hyphen, plus. No leading/trailing/consecutive dots.
            inline bool is_valid_local_part(const S& lp)
            {
                if (lp.empty() || lp.size() > 64) return false;
                // No leading or trailing dot.
                if (lp.front() == W('.') || lp.back() == W('.')) return false;
                bool prev_dot = false;
                for (const C ch : lp)
                {
                    if (ch == W('.'))
                    {
                        if (prev_dot) return false; // consecutive dots
                        prev_dot = true;
                        continue;
                    }
                    prev_dot = false;
                    // RFC 5322 allows a wider set, but for speed we accept the common ones.
                    if ((ch >= W('a') && ch <= W('z')) ||
                        (ch >= W('A') && ch <= W('Z')) ||
                        (ch >= W('0') && ch <= W('9')) ||
                        ch == W('_') || ch == W('-') || ch == W('+'))
                        continue;
                    return false;
                }
                return true;
            }

            /// \brief Validates the domain part of an email address.
            /// Simplified: labels separated by dots, alphanumeric + hyphen, no leading/trailing hyphen per label, TLD >= 2 chars.
            inline bool is_valid_domain(const S& domain)
            {
                if (domain.empty() || domain.size() > 255) return false;
                // Split into labels by '.'
                size_t start = 0;
                size_t label_count = 0;
                while (start <= domain.size())
                {
                    size_t dot = domain.find(W('.'), start);
                    if (dot == S::npos) dot = domain.size();
                    size_t len = dot - start;
                    if (len == 0 || len > 63) return false; // empty or oversized label
                    // No leading or trailing hyphen in a label.
                    if (domain[start] == W('-') || domain[dot - 1] == W('-')) return false;
                    for (size_t i = start; i < dot; ++i)
                    {
                        C ch = domain[i];
                        if ((ch >= W('a') && ch <= W('z')) ||
                            (ch >= W('A') && ch <= W('Z')) ||
                            (ch >= W('0') && ch <= W('9')) ||
                            ch == W('-'))
                            continue;
                        return false;
                    }
                    ++label_count;
                    start = dot + 1;
                }
                if (label_count < 2) return false; // need at least "host.tld"
                // TLD (last label) must be >= 2 characters and all-alpha.
                size_t last_dot = domain.rfind(W('.'));
                S tld = domain.substr(last_dot + 1);
                if (tld.size() < 2) return false;
                for (const C ch : tld)
                {
                    if (!((ch >= W('a') && ch <= W('z')) || (ch >= W('A') && ch <= W('Z'))))
                        return false;
                }
                return true;
            }

            /// \brief Validates a full email address (local@domain).
            inline bool is_valid_email_address(const S& email_address) 
            {
                if (email_address.empty()) return false;
                auto at = email_address.find(W('@'));
                if (at == S::npos || at == 0 || at == email_address.size() - 1) return false;
                // Only one '@' allowed.
                if (email_address.find(W('@'), at + 1) != S::npos) return false;
                return is_valid_local_part(email_address.substr(0, at)) &&
                       is_valid_domain(email_address.substr(at + 1));
            }

            struct Email
            {
                typedef pd::CS<0, 64> LocalPart; // The maximum length of the local part is 64 characters
                typedef pd::CS<0, 255> Domain; // The maximum length of the domain name is 255 characters    
                LocalPart mlocal_part;
                Domain    mdomain;
                ContactQualifier mqualifier;
                const inline static LocalPart NULL_LOCAL_PART = LocalPart(W("null"));
                const inline static Domain NULL_DOMAIN = Domain(W("null"));

                static inline constexpr const S null_email_str() noexcept
				{
					return W("null@null");
				}

                inline void initialize(const LocalPart& lp = NULL_LOCAL_PART, const Domain& d = NULL_DOMAIN, ContactQualifier cq = ContactQualifier::Business)
                {
                    // Skip validation for null sentinel values.
                    bool is_null = (lp == NULL_LOCAL_PART && d == NULL_DOMAIN);
                    if (!is_null)
                    {
                        if (!is_valid_local_part(lp))
                        {
                            runtime_error(W("Invalid local part"));
                        }
                        if (!is_valid_domain(d))
                        {
                            runtime_error(W("Invalid domain"));
                        }
                    }
                    memcpy (&mlocal_part, &lp, sizeof (lp));
                    memcpy (&mdomain    , &d , sizeof (d ));
                    mqualifier  = cq;
                }

                inline Email(const LocalPart& lp = NULL_LOCAL_PART, const Domain& d = NULL_DOMAIN, ContactQualifier cq = ContactQualifier::Business)
                    : mlocal_part(lp), mdomain(d), mqualifier(cq) 
                {
                    initialize(lp, d, cq);
                }

                inline Email(const S& s, ContactQualifier cq = ContactQualifier::Business)
					: mqualifier(cq)
				{
					auto at = s.find(W('@'));
					if (at != S::npos)
					{
                        initialize(s.substr(0, at), s.substr(at + 1), cq);
					}
					else
					{
						runtime_error(W("Invalid email address"));
					}
				}

                inline Email (const C* s, ContactQualifier cq = ContactQualifier::Business)
				     : Email(S(s), cq)
				{
				}

                // Conversion to S
                inline S str() const noexcept
				{
                    return  mlocal_part.to_string () + W("@") + mdomain;
				}

                // Implicit conversion to basic_string<C>
                inline operator S() const noexcept
                {
                    return str ();
                }

                inline bool equal_local_part(const Email& other) const noexcept
                {
                    return mlocal_part == other.mlocal_part;
                }

                inline bool equal_domain(const Email& other) const noexcept
                {
                    return mdomain == other.mdomain;
                }

                inline bool operator==(const Email& other) const noexcept
                {
                    return equal_local_part(other) && equal_domain(other);
                }

                inline bool operator!=(const Email& other) const noexcept
                {
                    return !(*this == other);
                }

            }; // struct Email
             
            static inline const Email NULL_EMAIL = { Email::NULL_LOCAL_PART, Email::NULL_DOMAIN, ContactQualifier::Business };

            // Make Email OutputStreamable.
            inline OutStream& operator<<(OutStream& os, const Email& e)
            {
                os << e.mlocal_part.str () << W("@") << e.mdomain;
                return os;
            }

            // Make Email InputStreamable.
            inline InStream& operator>>(InStream& is, Email& e)
            {
                S s;
                is >> s;
                auto at = s.find(W('@'));
                if (at != S::npos)
                {
                    e.mlocal_part = s.substr(0, at);
                    e.mdomain = s.substr(at + 1);
                }
                else
                {
                    runtime_error(W("Invalid email address"));
                }
                return is;
            }    
        }   // namespace contact
    }       // namespace cpplib
}		    // namespace pensar_digital
#endif // EMAIL_HPP
