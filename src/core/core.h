#ifndef CORE_H
#define CORE_H

#include <QSettings>

#define ofCore Core::instance()

class Core : public QObject
{
    Q_OBJECT

public:
    // This should only be used to acccess the signals, so it could
    // theoretically return an QObject *. For source compatibility
    // it returns a Core.
    static Core *instance();

    QSettings *settings();

    /*
    ActionManager *actionManager();
    MessageManager *messageManager();
    EditorManager *editorManager();
    ProgressManager *progressManager();
    ScriptManager *scriptManager();
    VariableManager *variableManager();
    VcsManager *vcsManager();
    MimeDatabase *mimeDatabase();
    HelpManager *helpManager();
    */

public slots:
    void shutdown();

signals:
    void coreAboutToOpen();
    void coreOpened();
    void coreAboutToClose();

private:
    Core();
    ~Core();
};

#endif // CORE_H
