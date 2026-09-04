#include "core/ContactManager.h"

#include "core/Session.h"
#include "network/ApiClient.h"

ContactManager::ContactManager(QObject *parent)
    : QObject(parent)
{
    ApiClient &api = ApiClient::instance();
    connect(&api, &ApiClient::friendsLoaded,
            this, &ContactManager::onFriendsLoaded);
    connect(&api, &ApiClient::usersSearchLoaded,
            this, &ContactManager::onSearchResultsLoaded);
    connect(&api, &ApiClient::friendRequestSent,
            this, &ContactManager::onFriendRequestSent);
    connect(&api, &ApiClient::contactRequestFailed,
            this, &ContactManager::onRequestError);
}

void ContactManager::loadFriends()
{
    ApiClient::instance().fetchFriends();
}

void ContactManager::searchUsers(const QString &username)
{
    const QString value = username.trimmed();
    if (!value.isEmpty()) {
        ApiClient::instance().searchUsers(value);
    }
}

void ContactManager::addFriend(qint64 userId, const QString &message)
{
    if (userId > 0) {
        ApiClient::instance().sendFriendRequest(userId, message);
    }
}

void ContactManager::onFriendsLoaded(const QList<UserSummary> &friends)
{
    if (Session::instance().isLoggedIn()) {
        emit friendsChanged(friends);
    }
}

void ContactManager::onSearchResultsLoaded(const QList<UserSummary> &users)
{
    if (Session::instance().isLoggedIn()) {
        emit searchResultsChanged(users);
    }
}

void ContactManager::onFriendRequestSent(const FriendRequest &request)
{
    if (Session::instance().isLoggedIn()) {
        emit friendRequestSent(request);
    }
}

void ContactManager::onRequestError(const QString &operation, const QString &message)
{
    if (Session::instance().isLoggedIn()) {
        emit requestError(operation, message);
    }
}
