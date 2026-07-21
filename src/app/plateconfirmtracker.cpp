#include "plateconfirmtracker.h"
bool PlateConfirmTracker::report(const QString &plate)
{
    QDateTime now = QDateTime::currentDateTime();

    ConfirmState &state = m_tracker[plate];

    if(state.lastAction.isValid()){
        int secsSinceAction = state.lastAction.secsTo(now);
        if(secsSinceAction < m_cooldownSecs){
            return false;
        }
    }

    if(state.lastSeen.isValid()){
        int secsSinceLast = state.lastSeen.secsTo(now);
        if(secsSinceLast > m_expirySecs){
            state.count = 0;
        }
    }

    if(state.count == 0){
        state.firstSeen = now;
    }
    state.lastSeen = now;
    state.count++;

    if(state.count >= m_confirmCount){
        state.count = 0;
        return true;
    }
    return false;
}
void PlateConfirmTracker::markActioned(const QString &plate)
{
    m_tracker[plate].lastAction = QDateTime::currentDateTime();
}