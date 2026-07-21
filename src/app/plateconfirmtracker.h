#ifndef PLATECONFIRMTRACKER_H
#define PLATECONFIRMTRACKER_H
#include <QString>
#include <QHash>
#include <QDateTime>
struct ConfirmState {
    int count = 0;
    QDateTime firstSeen;
    QDateTime lastSeen;
    QDateTime lastAction;
};
class PlateConfirmTracker
{
public:
    PlateConfirmTracker() = default;

    bool report(const QString &plate);

    void markActioned(const QString &plate);

    void setCooldownSecs(int secs) { m_cooldownSecs = secs;}

    void setConfirmCount(int count) { m_confirmCount = count;}

    void setExpirySecs(int secs) {m_expirySecs = secs;}
private:
    QHash<QString, ConfirmState> m_tracker;
    int m_cooldownSecs  = 30;
    int m_confirmCount  = 3;
    int m_expirySecs  = 5;
};
#endif