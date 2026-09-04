#pragma once

#include <QList>
#include <QObject>
#include <QString>

#include "core/models.h"

// 联系人领域管理器：封装好友列表、用户搜索和好友申请。
// 界面层只依赖本类，不直接调用 ApiClient；聊天会话仍由 ChatManager 管理。
class ContactManager : public QObject {
    Q_OBJECT

public:
    explicit ContactManager(QObject *parent = nullptr);

    void loadFriends();
    void searchUsers(const QString &username);
    void addFriend(qint64 userId, const QString &message = QString());

signals:
    void friendsChanged(const QList<UserSummary> &friends);
    void searchResultsChanged(const QList<UserSummary> &users);
    void friendRequestSent(const FriendRequest &request);
    void requestError(const QString &operation, const QString &message);

private slots:
    void onFriendsLoaded(const QList<UserSummary> &friends);
    void onSearchResultsLoaded(const QList<UserSummary> &users);
    void onFriendRequestSent(const FriendRequest &request);

private:
    void onRequestError(const QString &operation, const QString &message);
};
