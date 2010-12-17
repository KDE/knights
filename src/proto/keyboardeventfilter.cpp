/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "keyboardeventfilter.h"
#include <QEvent>
#include <QKeyEvent>
#include <KDebug>

using namespace Knights;

KeyboardEventFilter::KeyboardEventFilter ( QObject* parent ) : QObject ( parent ),
  pwMode(false)
{

}

KeyboardEventFilter::~KeyboardEventFilter()
{

}

bool KeyboardEventFilter::eventFilter ( QObject* object, QEvent* event )
{
    kDebug() << event->type() << QEvent::Shortcut << QEvent::ShortcutOverride << QEvent::KeyPress;
    if ( event->type() == QEvent::KeyRelease )
    {
	QKeyEvent* ke = static_cast<QKeyEvent*>(event);
	kDebug() << ke->key() << ke->modifiers();
	if ( ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return )
	{
	  emit enterPressed();
	}
	else if (!ke->text().isEmpty())
	{
	 emit textTyped(ke->text());
	 if ( pwMode )
	 {
	    return true;
	 }
	}
    }
    return QObject::eventFilter ( object, event );
}

void Knights::KeyboardEventFilter::setPasswordMode ( bool passwordMode )
{
  pwMode = passwordMode;
}

bool Knights::KeyboardEventFilter::passwordMade()
{
  return pwMode;
}
