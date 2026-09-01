#include "core/Session.h"

Session &Session::instance()
{
    static Session s_instance;
    return s_instance;
}

void Session::setLogin(const LoginResult &login)
{
    m_login = login;
}

void Session::clear()
{
    m_login = LoginResult{};
}

bool Session::isLoggedIn() const
{
    return !m_login.accessToken.isEmpty();
}

const LoginResult &Session::login() const
{
    return m_login;
}

const QString &Session::token() const
{
    return m_login.accessToken;
}
