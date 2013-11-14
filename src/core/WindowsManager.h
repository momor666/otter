#ifndef OTTER_WINDOWSMANAGER_H
#define OTTER_WINDOWSMANAGER_H

#include <QtCore/QUrl>
#include <QtPrintSupport/QPrinter>
#include <QtWidgets/QMdiArea>

namespace Otter
{

class StatusBarWidget;
class TabBarWidget;
class Window;

class WindowsManager : public QObject
{
	Q_OBJECT

public:
	explicit WindowsManager(QMdiArea *area, TabBarWidget *tabBar, StatusBarWidget *statusBar, bool privateSession = false);

	Window* getWindow(int index = -1) const;
	QString getTitle() const;
	int getCurrentWindow() const;
	int getZoom() const;
	bool canUndo() const;
	bool canRedo() const;

public slots:
	void open(const QUrl &url = QUrl(), bool privateWindow = false);
	void close(int index = -1);
	void closeOther(int index = -1);
	void print(int index = -1);
	void printPreview(int index = -1);
	void reload();
	void stop();
	void goBack();
	void goForward();
	void undo();
	void redo();
	void cut();
	void copy();
	void paste();
	void remove();
	void selectAll();
	void zoomIn();
	void zoomOut();
	void zoomOriginal();
	void setZoom(int zoom);

protected:
	int getWindowIndex(Window *window) const;

protected slots:
	void printPreview(QPrinter *printer);
	void addWindow(Window *window);
	void cloneWindow(int index);
	void pinWindow(int index, bool pin);
	void closeWindow(int index);
	void setCurrentWindow(int index);
	void setTitle(const QString &title);

private:
	QMdiArea *m_area;
	TabBarWidget *m_tabBar;
	StatusBarWidget *m_statusBar;
	int m_currentWindow;
	int m_printedWindow;
	bool m_privateSession;

signals:
	void windowAdded(int index);
	void windowRemoved(int index);
	void currentWindowChanged(int index);
	void windowTitleChanged(QString title);
	void undoTextChanged(QString undoText);
	void redoTextChanged(QString redoText);
	void canUndoChanged(bool canUndo);
	void canRedoChanged(bool canRedo);
};

}

#endif