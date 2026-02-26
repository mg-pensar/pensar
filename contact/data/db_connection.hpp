// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#ifndef CONTACT_DB_CONNECTION_HPP
#define CONTACT_DB_CONNECTION_HPP

#include <soci/soci.h>
#include <soci/postgresql/soci-postgresql.h>
#include <soci/connection-pool.h>

#include <string>
#include <memory>
#include <stdexcept>
#include <mutex>

namespace pensar_digital::cpplib::contact::db
{

/// \brief Database connection manager using SOCI connection pool.
///
/// Provides a RAII-safe scoped session that propagates `app.current_user`
/// and `app.locale` session variables for audit triggers and locale-aware
/// accessor functions.
///
/// Usage:
/// \code
///     DbConnection pool("dbname=contact_db user=mg", 4);
///     {
///         auto session = pool.session("admin_user", "en-US");
///         session.sql() << "INSERT INTO t_person ...";
///     } // session returned to pool
/// \endcode
class DbConnection
{
public:
    /// \brief Scoped session — borrows a connection from the pool and
    ///        sets session variables. Returns to pool on destruction.
    class ScopedSession
    {
    public:
        ScopedSession(soci::connection_pool& pool,
                      const std::string& username,
                      const std::string& locale)
            : pool_(pool)
            , pos_(pool.lease())
            , session_(pool_.at(pos_))
        {
            // Propagate app.current_user for audit triggers
            if (!username.empty())
            {
                session_ << "SET \"app.current_user\" = "
                            + quote_literal(username);
            }
            // Propagate app.locale for locale-aware accessor functions
            if (!locale.empty())
            {
                session_ << "SET \"app.locale\" = "
                            + quote_literal(locale);
            }
        }

        ~ScopedSession()
        {
            try {
                session_ << "RESET \"app.current_user\"";
                session_ << "RESET \"app.locale\"";
            } catch (...) {}
            pool_.give_back(pos_);
        }

        // Non-copyable, non-movable
        ScopedSession(const ScopedSession&)            = delete;
        ScopedSession& operator=(const ScopedSession&) = delete;
        ScopedSession(ScopedSession&&)                 = delete;
        ScopedSession& operator=(ScopedSession&&)      = delete;

        soci::session& sql() noexcept { return session_; }

    private:
        /// SQL-safe quoting: escapes single quotes by doubling them.
        static std::string quote_literal(const std::string& s)
        {
            std::string result = "'";
            for (char c : s)
            {
                if (c == '\'') result += "''";
                else           result += c;
            }
            result += '\'';
            return result;
        }

        soci::connection_pool& pool_;
        std::size_t            pos_;
        soci::session&         session_;
    };

    /// \param conn_string  SOCI connection string, e.g.
    ///        "dbname=contact_db user=mg host=localhost port=5432"
    /// \param pool_size    Number of pooled connections (default: 4)
    explicit DbConnection(const std::string& conn_string,
                          std::size_t pool_size = 4)
        : pool_(pool_size)
    {
        for (std::size_t i = 0; i < pool_size; ++i)
        {
            auto& s = pool_.at(i);
            s.open(soci::postgresql, conn_string);
        }
    }

    ~DbConnection() = default;

    // Non-copyable, non-movable (connection_pool is non-movable)
    DbConnection(const DbConnection&)            = delete;
    DbConnection& operator=(const DbConnection&) = delete;
    DbConnection(DbConnection&&)                 = delete;
    DbConnection& operator=(DbConnection&&)      = delete;

    /// Borrow a session from the pool with session variable propagation.
    [[nodiscard]]
    ScopedSession session(const std::string& username = {},
                          const std::string& locale   = {})
    {
        return ScopedSession(pool_, username, locale);
    }

    soci::connection_pool& pool() noexcept { return pool_; }

private:
    soci::connection_pool pool_;
};

} // namespace pensar_digital::cpplib::contact::db

#endif // CONTACT_DB_CONNECTION_HPP
