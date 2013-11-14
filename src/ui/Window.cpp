#include "Window.h"
#include "../core/ActionsManager.h"
#include "../core/NetworkAccessManager.h"

#include "ui_Window.h"

#include <QtCore/QBuffer>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QMimeDatabase>
#include <QtWebKitWidgets/QWebFrame>
#include <QtWebKitWidgets/QWebPage>
#include <QtGui/QGuiApplication>

namespace Otter
{

Window::Window(QWidget *parent) : QWidget(parent),
	m_isLoading(false),
	m_isPinned(false),
	m_ui(new Ui::Window)
{
	m_ui->setupUi(this);
	m_ui->backButton->setDefaultAction(getAction(GoBackAction));
	m_ui->forwardButton->setDefaultAction(getAction(GoForwardAction));
	m_ui->reloadButton->setDefaultAction(getAction(ReloadAction));
	m_ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
	m_ui->webView->page()->setNetworkAccessManager(new NetworkAccessManager(this));

	ActionsManager::setupLocalAction(getAction(GoBackAction), "GoBack");
	ActionsManager::setupLocalAction(getAction(GoForwardAction), "GoForward");
	ActionsManager::setupLocalAction(getAction(ReloadAction), "Reload");
	ActionsManager::setupLocalAction(getAction(StopAction), "Stop");

	connect(m_ui->backButton, SIGNAL(clicked()), this, SLOT(goBack()));
	connect(m_ui->forwardButton, SIGNAL(clicked()), this, SLOT(goForward()));
	connect(m_ui->reloadButton, SIGNAL(clicked()), this, SLOT(reload()));
	connect(m_ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(loadUrl()));
	connect(m_ui->webView, SIGNAL(titleChanged(const QString)), this, SLOT(notifyTitleChanged()));
	connect(m_ui->webView, SIGNAL(urlChanged(const QUrl)), this, SLOT(notifyUrlChanged(const QUrl)));
	connect(m_ui->webView, SIGNAL(iconChanged()), this, SLOT(notifyIconChanged()));
	connect(m_ui->webView->page(), SIGNAL(linkClicked(QUrl)), this, SLOT(setUrl(QUrl)));
	connect(m_ui->webView->page(), SIGNAL(loadStarted()), this, SLOT(loadStarted()));
	connect(m_ui->webView->page(), SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
}

Window::~Window()
{
	delete m_ui;
}

void Window::print(QPrinter *printer)
{
	m_ui->webView->print(printer);
}

Window* Window::clone(QWidget *parent)
{
	if (!isClonable())
	{
		return NULL;
	}

	Window *window = new Window(parent);
	window->setPinned(isPinned());
	window->setPrivate(isPrivate());
	window->setUrl(getUrl());
	window->setZoom(getZoom());

///TODO clone history

	return window;
}

QAction *Window::getAction(WebAction action)
{
	QWebPage::WebAction webAction = mapAction(action);

	if (webAction != QWebPage::NoWebAction)
	{
		return m_ui->webView->page()->action(webAction);
	}

	return NULL;
}

void Window::changeEvent(QEvent *event)
{
	QWidget::changeEvent(event);

	switch (event->type())
	{
		case QEvent::LanguageChange:
			m_ui->retranslateUi(this);

			break;
		default:
			break;
	}
}

void Window::reload()
{
	m_ui->webView->page()->triggerAction(QWebPage::Reload);
}

void Window::stop()
{
	m_ui->webView->page()->triggerAction(QWebPage::Stop);
}

void Window::goBack()
{
	m_ui->webView->page()->triggerAction(QWebPage::Back);
}

void Window::goForward()
{
	m_ui->webView->page()->triggerAction(QWebPage::Forward);
}

void Window::undo()
{
	m_ui->webView->page()->undoStack()->undo();
}

void Window::redo()
{
	m_ui->webView->page()->undoStack()->redo();
}

void Window::cut()
{
	m_ui->webView->page()->triggerAction(QWebPage::Cut);
}

void Window::copy()
{
	m_ui->webView->page()->triggerAction(QWebPage::Copy);
}

void Window::paste()
{
	m_ui->webView->page()->triggerAction(QWebPage::Paste);
}

void Window::remove()
{
	m_ui->webView->page()->triggerAction(QWebPage::DeleteEndOfWord);
}

void Window::selectAll()
{
	m_ui->webView->page()->triggerAction(QWebPage::SelectAll);
}

void Window::zoomIn()
{
	setZoom(qMin((getZoom() + 10), 10000));
}

void Window::zoomOut()
{
	setZoom(qMax((getZoom() - 10), 10));
}

void Window::zoomOriginal()
{
	setZoom(100);
}

void Window::setZoom(int zoom)
{
	if (zoom != getZoom())
	{
		m_ui->webView->setZoomFactor(qBound(0.1, ((qreal) zoom / 100), (qreal) 100));

		emit zoomChanged(zoom);
	}
}

void Window::setUrl(const QUrl &url)
{
	if (url.isValid() && url.scheme().isEmpty() && !url.path().startsWith('/'))
	{
		QUrl httpUrl = url;
		httpUrl.setScheme("http");

		m_ui->webView->setUrl(httpUrl);
	}
	else if (url.isValid() && (url.scheme().isEmpty() || url.scheme() == "file"))
	{
		QUrl localUrl = url;
		localUrl.setScheme("file");

		if (localUrl.isLocalFile() && QFileInfo(localUrl.toLocalFile()).isDir())
		{
			QFile file(":/files/listing.html");
			file.open(QIODevice::ReadOnly | QIODevice::Text);

			QTextStream stream(&file);
			stream.setCodec("UTF-8");

			QDir directory(localUrl.toLocalFile());
			const QFileInfoList entries = directory.entryInfoList((QDir::AllEntries | QDir::Hidden), (QDir::Name | QDir::DirsFirst));
			QStringList navigation;

			do
			{
				navigation.prepend(QString("<a href=\"%1\">%2</a>%3").arg(directory.canonicalPath()).arg(directory.dirName().isEmpty() ? QString('/') : directory.dirName()).arg(directory.dirName().isEmpty() ? QString() : QString('/')));
			}
			while (directory.cdUp());

			QHash<QString, QString> variables;
			variables["title"] = QFileInfo(localUrl.toLocalFile()).canonicalFilePath();
			variables["description"] = tr("Directory Contents");
			variables["dir"] = (QGuiApplication::isLeftToRight() ? "ltr" : "rtl");
			variables["navigation"] = navigation.join(QString());
			variables["header_name"] = tr("Name");
			variables["header_type"] = tr("Type");
			variables["header_size"] = tr("Size");
			variables["header_date"] = tr("Date");
			variables["body"] = QString();

			QMimeDatabase database;

			for (int i = 0; i < entries.count(); ++i)
			{
				const QMimeType mimeType = database.mimeTypeForFile(entries.at(i).canonicalFilePath());
				QString size;

				if (!entries.at(i).isDir())
				{
					if (entries.at(i).size() > 1024)
					{
						if (entries.at(i).size() > 1048576)
						{
							if (entries.at(i).size() > 1073741824)
							{
								size = QString("%1 GB").arg((entries.at(i).size() / 1073741824.0), 0, 'f', 2);
							}
							else
							{
								size = QString("%1 MB").arg((entries.at(i).size() / 1048576.0), 0, 'f', 2);
							}
						}
						else
						{
							size = QString("%1 KB").arg((entries.at(i).size() / 1024.0), 0, 'f', 2);
						}
					}
					else
					{
						size = QString("%1 B").arg(entries.at(i).size());
					}
				}

				QByteArray byteArray;
				QBuffer buffer(&byteArray);
				QIcon::fromTheme(mimeType.iconName(), QIcon(entries.at(i).isDir() ? ":icons/inode-directory.png" : ":/icons/unknown.png")).pixmap(16, 16).save(&buffer, "PNG");

				variables["body"].append(QString("<tr>\n<td><a href=\"file://%1\"><img src=\"data:image/png;base64,%2\" alt=\"\"> %3</a></td>\n<td>%4</td>\n<td>%5</td>\n<td>%6</td>\n</tr>\n").arg(entries.at(i).filePath()).arg(QString(byteArray.toBase64())).arg(entries.at(i).fileName()).arg(mimeType.comment()).arg(size).arg(entries.at(i).lastModified().toString()));
			}

			QString html = stream.readAll();
			QHash<QString, QString>::iterator iterator;

			for (iterator = variables.begin(); iterator != variables.end(); ++iterator)
			{
				html.replace(QString("{%1}").arg(iterator.key()), iterator.value());
			}

			m_ui->webView->setHtml(html, localUrl);
		}
		else
		{
			m_ui->webView->setUrl(localUrl);
		}
	}
	else
	{
		m_ui->webView->setUrl(url);
	}

	notifyTitleChanged();
	notifyIconChanged();
}

void Window::setPinned(bool pinned)
{
	if (pinned != m_isPinned)
	{
		m_isPinned = pinned;

		emit isPinnedChanged(pinned);
	}
}

void Window::setPrivate(bool enabled)
{
	if (enabled != m_ui->webView->settings()->testAttribute(QWebSettings::PrivateBrowsingEnabled))
	{
		m_ui->webView->settings()->setAttribute(QWebSettings::PrivateBrowsingEnabled, enabled);

		notifyIconChanged();

		emit isPrivateChanged(enabled);
	}
}

void Window::loadUrl()
{
	setUrl(QUrl(m_ui->lineEdit->text()));
}

void Window::loadStarted()
{
	m_isLoading = true;

	emit loadingChanged(true);
}

void Window::loadFinished(bool ok)
{
	Q_UNUSED(ok)

	m_isLoading = false;

	emit loadingChanged(false);
}

void Window::notifyTitleChanged()
{
	emit titleChanged(getTitle());
}

void Window::notifyUrlChanged(const QUrl &url)
{
	m_ui->lineEdit->setText(url.toString());

	emit urlChanged(url);
}

void Window::notifyIconChanged()
{
	emit iconChanged(getIcon());
}

QUndoStack *Window::getUndoStack()
{
	return m_ui->webView->page()->undoStack();
}

QString Window::getTitle() const
{
	const QString title = m_ui->webView->title();

	if (title.isEmpty())
	{
		if (isEmpty())
		{
			return tr("New Tab");
		}

		const QUrl url = getUrl();

		if (url.isLocalFile())
		{
			return QFileInfo(url.toLocalFile()).canonicalFilePath();
		}

		return tr("Empty");
	}

	return title;
}

QUrl Window::getUrl() const
{
	return m_ui->webView->url();
}

QIcon Window::getIcon() const
{
	if (isPrivate())
	{
		return QIcon(":/icons/tab-private.png");
	}

	const QIcon icon = m_ui->webView->icon();

	return (icon.isNull() ? QIcon(":/icons/tab.png") : icon);
}

QWebPage::WebAction Window::mapAction(WebAction action) const
{
	switch (action)
	{
		case OpenLinkAction:
			return QWebPage::OpenLink;
		case OpenLinkInNewWindowAction:
			return QWebPage::OpenLinkInNewWindow;
		case OpenLinkInThisWindowAction:
			return QWebPage::OpenLinkInThisWindow;
		case OpenFrameInNewWindowAction:
			return QWebPage::OpenFrameInNewWindow;
		case DownloadLinkToDiskAction:
			return QWebPage::DownloadLinkToDisk;
		case CopyLinkToClipboardAction:
			return QWebPage::CopyLinkToClipboard;
		case OpenImageInNewWindowAction:
			return QWebPage::OpenImageInNewWindow;
		case DownloadImageToDiskAction:
			return QWebPage::DownloadImageToDisk;
		case CopyImageToClipboardAction:
			return QWebPage::CopyImageToClipboard;
		case CopyImageUrlToClipboardAction:
			return QWebPage::CopyImageUrlToClipboard;
		case GoBackAction:
			return QWebPage::Back;
		case GoForwardAction:
			return QWebPage::Forward;
		case StopAction:
			return QWebPage::Stop;
		case StopScheduledPageRefreshAction:
			return QWebPage::StopScheduledPageRefresh;
		case ReloadAction:
			return QWebPage::Reload;
		case ReloadAndBypassCacheAction:
			return QWebPage::ReloadAndBypassCache;
		case CutAction:
			return QWebPage::Cut;
		case CopyAction:
			return QWebPage::Copy;
		case PasteAction:
			return QWebPage::Paste;
		case DeleteAction:
			return QWebPage::DeleteEndOfWord;
		case SelectAllAction:
			return QWebPage::SelectAll;
		case UndoAction:
			return QWebPage::Undo;
		case RedoAction:
			return QWebPage::Redo;
		case InspectElementAction:
			return QWebPage::InspectElement;
		default:
			return QWebPage::NoWebAction;
	}

	return QWebPage::NoWebAction;
}

int Window::getZoom() const
{
	return (m_ui->webView->zoomFactor() * 100);
}

bool Window::isClonable() const
{
	return true;
}

bool Window::isEmpty() const
{
	const QUrl url = m_ui->webView->url();

	return (url.scheme() == "about" && (url.path().isEmpty() || url.path() == "blank"));
}

bool Window::isLoading() const
{
	return m_isLoading;
}

bool Window::isPinned() const
{
	return m_isPinned;
}

bool Window::isPrivate() const
{
	return m_ui->webView->settings()->testAttribute(QWebSettings::PrivateBrowsingEnabled);
}

}