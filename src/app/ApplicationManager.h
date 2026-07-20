#ifndef APPLICATIONMANAGER
#define APPLICATIONMANAGER

#include <QObject>
#include "src/database/databasemanager.h"

class LoginDialog;
class MainWindow;

class ApplicationManager : public QObject {
    Q_OBJECT

public:
    explicit ApplicationManager(DatabaseManager &dbMgr, QObject *parent = nullptr);
    ~ApplicationManager();

    // 应用入口：创建窗口、建立信号连接、显示登录框。
    // 返回后由 main() 调 app.exec() 驱动全局事件循环。
    void start();

private slots:
    // 登录成功：隐藏登录框，显示主窗口
    void onLoginAccepted();
    // 退出登录：隐藏主窗口，回到登录框
    void onLogoutRequested();
    // 关闭按钮：退出整个应用
    void onAppExitRequested();

private:
    DatabaseManager &m_dbMgr;
    LoginDialog *m_loginDialog = nullptr;
    MainWindow  *m_mainWindow  = nullptr;
};

#endif // APPLICATIONMANAGER

/*
 * ═══════════════════════════════════════════════════════════════════════════
 *  ApplicationManager 设计说明（信号驱动架构 v2）
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * 【为什么放弃旧的 QEventLoop 嵌套循环方案？】
 *
 *  旧方案在 run() 里用 while + 局部 QEventLoop 阻塞等待窗口关闭。
 *  实测无法稳定工作，根因有二：
 *
 *  1. QApplication 默认 quitOnLastWindowClosed=true。
 *     主窗口 close() 的瞬间没有可见顶层窗口 → Qt 发 lastWindowClosed
 *     → 触发 QCoreApplication::quit() → 整个 app 被标记为退出中，
 *     后续 LoginDialog::exec() 再也起不来 → 程序卡死。
 *
 *  2. QEventLoop 嵌套 + close()/destroyed 的时序极其脆弱。
 *     任何一处 close() 被拦截、延迟或拒绝，loop.exec() 就永远退不出。
 *
 *  两个问题叠加，导致「点击退出登录后无反应」。
 *
 * 【新架构：信号驱动 + 窗口常驻】
 *
 *  - LoginDialog 和 MainWindow 都在 start() 里 new 出来，长生命周期常驻。
 *  - 切换窗口只调 hide()/show()，【绝不 close()、绝不 delete】。
 *    → 没有任何顶层窗口被关闭，lastWindowClosed 永远不触发，
 *      规避了 quitOnLastWindowClosed 的所有陷阱。
 *  - 所有状态转换由信号驱动：
 *      LoginDialog::accepted       → 显示主窗口
 *      MainWindow::logoutRequested → 回登录框
 *      MainWindow::appExitRequested→ qApp->quit()
 *  - main() 只需 start() 一次，然后 app.exec() 走标准 Qt 流程。
 *
 * 【退出语义】
 *
 *  - 退出登录：只是窗口切换，应用不退出。
 *  - 退出应用：主窗口关闭按钮 → emit appExitRequested → qApp->quit()。
 *    app.exec() 返回后，栈上的 ApplicationManager 析构，
 *    两个窗口作为其子对象被 Qt 自动 delete，无泄漏。
 *
 * 【为什么主窗口不能 close()?】
 *
 *  主窗口关闭按钮的语义被重新定义为「退出整个应用」，不是「关闭窗口」。
 *  所以 onCloseButton 不调 close()，只 emit 信号，
 *  真正的退出由 ApplicationManager 通过 qApp->quit() 完成。
 *
 * 【关于 quitOnLastWindowClosed】
 *
 *  本架构下登录框和主窗口始终至少有一个存活（只是可能被 hide），
 *  从 Qt 角度看「最后一个 visible 窗口关闭」仍可能触发该默认行为。
 *  因此 main.cpp 里必须保留 app.setQuitOnLastWindowClosed(false)，
 *  退出完全交由 appExitRequested 信号主动控制。
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */
