
#ifndef NEDIT_SERVER_H_
#define NEDIT_SERVER_H_

#include <QObject>

class QLocalServer;
class QString;

class NeditServer : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-BUS Interface", "com.nedit.IServer")

public:
    explicit NeditServer(QObject *parent = Q_NULLPTR);
    virtual ~NeditServer() = default;

public Q_SLOTS:
    void processCommand(const QString &command);
    void newConnection();

private:
    QLocalServer *server_;


};

#endif
