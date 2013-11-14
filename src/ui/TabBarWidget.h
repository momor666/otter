#ifndef OTTER_TABBARWIDGET_H
#define OTTER_TABBARWIDGET_H

#include <QtWidgets/QTabBar>

namespace Otter
{

class TabBarWidget : public QTabBar
{
	Q_OBJECT

public:
	explicit TabBarWidget(QWidget *parent = NULL);

	QVariant getTabProperty(int index, const QString &key, const QVariant &defaultValue) const;

public slots:
	void updateTabs(int index = -1);
	void setOrientation(Qt::DockWidgetArea orientation);
	void setShape(QTabBar::Shape shape);
	void setTabProperty(int index, const QString &key, const QVariant &value);

protected:
	void contextMenuEvent(QContextMenuEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void leaveEvent(QEvent *event);
	void resizeEvent(QResizeEvent *event);
	void tabInserted(int index);
	void tabRemoved(int index);
	QSize tabSizeHint(int index) const;
	QSize getTabSize(bool isHorizontal) const;

protected slots:
	void closeOther();
	void cloneTab();
	void pinTab();

private:
	QSize m_tabSize;
	int m_clickedTab;

signals:
	void requestedClone(int index);
	void requestedPin(int index, bool pin);
	void requestedClose(int index);
	void requestedCloseOther(int index);
};

}

#endif