/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2013 - 2014 Michal Dutkiewicz aka Emdek <michal@emdek.pl>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
**************************************************************************/

#include "ActionWidget.h"
#include "ContentsWidget.h"
#include "Window.h"

namespace Otter
{

ActionWidget::ActionWidget(ActionIdentifier identifier, Window *window, QWidget *parent) : QToolButton(parent)
{
	setAutoRaise(true);

	if (window && window->getContentsWidget()->getAction(identifier))
	{
		setDefaultAction(window->getContentsWidget()->getAction(identifier));
	}
	else
	{
		setDefaultAction(ActionsManager::getAction(identifier, this));
	}
}

void ActionWidget::enterEvent(QEvent *event)
{
	QToolButton::enterEvent(event);

	if (defaultAction() && !defaultAction()->shortcut().isEmpty())
	{
		setToolTip(QStringLiteral("%1 (%2)").arg(defaultAction()->text()).arg(defaultAction()->shortcut().toString(QKeySequence::NativeText)));
	}
}

}
