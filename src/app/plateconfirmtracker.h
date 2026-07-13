#ifndef PLATECONFIRMTRACKER_H
#define PLATECONFIRMTRACKER_H

#include <QString>
#include <QHash>
#include <QDateTime>

/**
 * @brief 单个车牌的确认状态
 */
struct ConfirmState {
    int count = 0;// 连续识别次数
    QDateTime firstSeen; // 本轮首次识别时间
    QDateTime lastSeen;  // 最近一次识别时间
    QDateTime lastAction;// 上次出入库操作完成时间
};

/**
 * @brief 车牌防重复校验器
 *
 * 在主线程中使用，接收来自 RecognizeThread 的识别结果，
 * 通过双重保护机制（冷却时间 + 多次确认）决定是否触发出入库操作。
 *
 * 使用方式：
 *   1. 每次收到识别结果 → 调用 report(plate)
 *   2. report 返回 true → 触发出入库操作
 *   3. 操作完成后 → 调用 markActioned(plate) 记录冷却起点
 *
 * 校验流程图：
 *
 *   report(plate)
 *     │
 *     ├─ 第1层：冷却时间
 *     │  lastAction 距今 < cooldownSecs？
 *     │  ├── 是 → return false（静默忽略）
 *     │  └── 否 → 进入第2层
 *     │
 *     └─ 第2层：多次确认
 *        lastSeen 距今 > expirySecs？
 *        ├── 是 → count = 1（连续性中断，重新开始）
 *        └── 否 → count++
 *        count >= confirmCount？
 *        ├── 否 → return false（等待更多确认）
 *        └── 是 → count = 0, return true（确认通过！）
 */
class PlateConfirmTracker
{
public:
    PlateConfirmTracker() = default;

    /**
     * @brief 报告一次识别结果
     *
     * 每次 RecognizeThread 识别到车牌后调用此方法。
     * 内部进行双重校验，只在两层都通过时返回 true。
     *
     * @param plate 识别到的车牌号
     * @return true = 确认通过，应触发出入库；false = 忽略
     */
    bool report(const QString &plate);

    /**
     * @brief 标记操作已完成
     *
     * 在 checkIn/checkOut 成功后调用，记录 lastAction 时间。
     * 后续 report() 会检查冷却时间，在冷却期内忽略该车牌。
     *
     * @param plate 完成操作的车牌号
     */
    void markActioned(const QString &plate);

    // ===== 配置方法 =====

    /** 设置冷却时间（秒），默认 30 */
    void setCooldownSecs(int secs) { m_cooldownSecs = secs;}

    /** 设置确认所需的连续识别次数，默认 3 */
    void setConfirmCount(int count) { m_confirmCount = count;}

    /** 设置连续性超时（秒），超过此时间未再次识别则重置计数，默认 5 */
    void setExpirySecs(int secs) {m_expirySecs = secs;}

private:
    /**
     * QHash<车牌号, 确认状态>
     *
     * 每个车牌独立追踪。不同车牌互不影响——
     * 摄像头交替识别到 A 和 B 时，A 的计数不会被 B 重置。
     */
    QHash<QString, ConfirmState> m_tracker;
    int m_cooldownSecs  = 30;// 冷却时间（秒）
    int m_confirmCount  = 3;// 确认所需连续次数
    int m_expirySecs  = 5;// 连续性超时（秒）
};

#endif