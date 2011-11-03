/*
    This file is part of Knights, a chess board for KDE SC 4.
    Copyright 2009,2010,2011  Miha Čančula <miha@noughmad.eu>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "proto/protocol.h"
#include "proto/chatwidget.h"
#include "core/move.h"

#include <KLocale>

#include <QtCore/QStack>
#include <QtCore/QTime>
#include <KDebug>
#include <gamemanager.h>

namespace Knights
{
    const int TimerInterval = 100; // milliseconds
    int id = qRegisterMetaType<Protocol::ErrorCode> ( "Protocol::ErrorCode" );

    QPointer<Protocol> Protocol::m_white = 0;
    QPointer<Protocol> Protocol::m_black = 0;

    class ProtocolPrivate
    {
        public:

            ProtocolPrivate();
            
            QVariantMap attributes;
            Protocol* white;
            Protocol* black;
            Color color;
    bool ready;
    int nextId;
    };

    ProtocolPrivate::ProtocolPrivate()
    : white(0)
    , black(0)
    , ready(false)
    , nextId(0)
    {

    }


    Protocol::Protocol ( QObject* parent ) : QObject ( parent ), d_ptr ( new ProtocolPrivate )
    {
    }

    Protocol::~Protocol()
    {
         delete d_ptr;
    }

    QString Protocol::stringFromErrorCode ( Protocol::ErrorCode code )
    {
        switch ( code )
        {
            case NoError:
                return i18n ( "No Error" );

            case UserCancelled:
                return i18n ( "User Canceled" );

            case NetworkError:
                return i18n ( "Network Error" );

            case UnknownError:
                return i18n ( "Unknown Error" );

            case InstallationError:
                return i18n ( "Program Error" );

            default:
                return QString();
        }
    }

void Protocol::setWhiteProtocol(Protocol* p)
{
    p->setColor(White);
    m_white = p;
}

void Protocol::setBlackProtocol(Protocol* p)
{
    p->setColor(Black);
    m_black = p;
}

Protocol* Protocol::white()
{
    return m_white;
}

Protocol* Protocol::black()
{
    return m_black;
}

Protocol* Protocol::byColor(Color color)
{
    switch ( color )
    {
        case White:
            return white();
        case Black:
            return black();
        case NoColor:
            return 0;
    }
    return 0;
}
    void Protocol::setColor ( Color color )
    {
        Q_D(Protocol);
        d->color = color;
    }

    Color Protocol::color() const
    {
        Q_D(const Protocol);
        return d->color;
    }

    void Protocol::setPlayerName ( const QString& name )
    {
        setAttribute ( QLatin1String ( "PlayerName" ), name );
    }

    QString Protocol::playerName() const
    {
        return attribute ( QLatin1String ( "PlayerName" ) ).toString();
    }

    void Protocol::setAttribute ( const QString& attribute, QVariant value )
    {
        Q_D ( Protocol );
        d->attributes.insert ( attribute,  value );
    }

    void Protocol::setAttribute ( const char* attribute, QVariant value )
    {
        setAttribute( QLatin1String ( attribute ), value );
    }

    void Protocol::setAttributes ( QVariantMap attributes )
    {
        Q_D ( Protocol );
        d->attributes.unite ( attributes );
    }

    QVariant Protocol::attribute ( const QString& attribute ) const
    {
        Q_D ( const Protocol );
        return d->attributes.value ( attribute );
    }

    QVariant Protocol::attribute ( const char* attribute ) const
    {
        return this->attribute ( QLatin1String ( attribute ) );
    }


    Protocol::Features Protocol::supportedFeatures()
    {
        return NoFeatures;
    }

    int Protocol::timeRemaining()
    {
        return -1;
    }

    QList< Protocol::ToolWidgetData > Protocol::toolWidgets()
    {
        return QList< Protocol::ToolWidgetData >();
    }
    
void Protocol::setWinner(Color winner)
{
    Q_UNUSED(winner);
}

void Protocol::setTimeControl(const TimeControl& c)
{
    Q_UNUSED(c);
}

void Protocol::acceptOffer(const Offer& offer)
{
    Q_UNUSED(offer);
}

void Protocol::declineOffer(const Offer& offer)
{
    Q_UNUSED(offer);
}

ChatWidget* Protocol::createChatWidget()
{
    return new ChatWidget;
}

ChatWidget* Protocol::createConsoleWidget()
{
    ChatWidget* console = new ChatWidget;
    console->setConsoleMode(true);
    return console;
}

void Protocol::initComplete()
{
    Q_D(Protocol);
    d->ready = true;
    emit initSuccesful();
}

bool Protocol::isReady()
{
    Q_D(const Protocol);
    return d->ready;
}

bool Protocol::isLocal()
{
    return false;
}

bool Protocol::isComputer()
{
    return false;
}

void Protocol::setDifficulty(int depth, int memory)
{
    Q_UNUSED(depth);
    Q_UNUSED(memory);
}

}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
