#ifndef MESSAGETYPE
#define MESSAGETYPE
#include <QObject>
class MessageType : public QObject {
    Q_OBJECT
public:
    enum class Type : int {
        Success      = 0,
        Failure      = 1,
        Info         = 2,
        ToastWarning = 3,
        Warning      = 4,
        Error        = 5,
        Confirm      = 6,
    };
    Q_ENUM(Type)
};
#endif
 MESSAGETYPE