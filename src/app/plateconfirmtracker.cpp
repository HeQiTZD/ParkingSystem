#include "plateconfirmtracker.h"

/*
 * report() 完整执行示例：
 *
 * 假设 confirmCount=3, cooldownSecs=30, expirySecs=5
 *
 * 时间轴：
 *   T=0s    识别到 苏A12345 → count=1, return false (需要3次)
 *   T=1.5s  识别到 苏A12345 → count=2, return false (需要3次)
 *   T=3.0s  识别到 苏A12345 → count=3, return true  (确认通过！触发入库)
 *           → 入库成功后调用 markActioned("苏A12345"), lastAction=T=3.0s
 *   T=4.5s  识别到 苏A12345 → 冷却中(距lastAction仅1.5s < 30s), return false
 *   T=35.0s 识别到 苏A12345 → 冷却已过, count=1, return false (重新计数)
 *
 * 连续性中断示例：
 *   T=0s    识别到 苏A12345 → count=1
 *   T=1.5s  识别到 苏A12345 → count=2
 *   T=8.0s  识别到 苏A12345 → 距上次 6.5s > 5s, 连续性中断, count=1 (重新开始)
 */

bool PlateConfirmTracker::report(const QString &plate)
{
    QDateTime now = QDateTime::currentDateTime();

    // 取或创建该车牌的追踪状态
    ConfirmState &state = m_tracker[plate];

    // ===== 第1层：冷却时间检查 =====
    // 如果该车牌最近刚完成过出入库操作，在冷却期内不再触发
    if(state.lastAction.isValid()){
        int secsSinceAction = state.lastAction.secsTo(now);
        if(secsSinceAction < m_cooldownSecs){
            return false;
        }
    }

    // ===== 第2层：多次确认 =====
    // 检查连续性：如果距离上次识别超过 expirySecs，说明车辆可能已离开又回来，
    // 需要重新开始计数
    if(state.lastSeen.isValid()){
        int secsSinceLast = state.lastSeen.secsTo(now);
        if(secsSinceLast > m_expirySecs){
            // 连续性中断 → 重置为第 1 次
            state.count = 0;
        }
    }

    // 更新时间戳
    if(state.count == 0){
        state.firstSeen = now; // 记录本轮首次识别时间
    }
    state.lastSeen = now;
    state.count++;

    // 检查是否达到确认阈值
    if(state.count >= m_confirmCount){
        // 确认通过！重置计数（下次需要重新累计）
        state.count = 0;
        return true;
    }

    return false;
}

void PlateConfirmTracker::markActioned(const QString &plate)
{
    // 记录操作完成时间 → 后续 report() 会进入冷却期
    m_tracker[plate].lastAction = QDateTime::currentDateTime();
}
