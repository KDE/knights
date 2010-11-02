/*
    This file is part of Knights, a chess board for KDE SC 4.
    Copyright 2009-2010  Miha Čančula <miha.cancula@gmail.com>

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

#ifndef KNIGHTS_PROTOCOL_H
#define KNIGHTS_PROTOCOL_H

#include <core/move.h>
#include <core/piece.h>

#include <QtCore/QObject>

namespace Knights
{
    class ProtocolPrivate;

    class Protocol : public QObject
    {
            Q_OBJECT
            Q_ENUMS ( Feature )
            Q_ENUMS ( ErrorCode )
            Q_FLAGS ( Features )
            Q_PROPERTY ( Color playerColor READ playerColor WRITE setPlayerColor )
            Q_PROPERTY ( QString opponentName READ opponentName )
            Q_PROPERTY ( QString playerName READ playerName )

        public:
            enum Feature
            {
                NoFeatures = 0x00, /**< The protocol supports none of the optional features */
                TimeLimit = 0x01, /**< The protocol supports setting a time limit for players */
                SetTimeLimit = 0x02, /**< The protocol can enable/disable and set the time limits */
                UpdateTime = 0x04, /**< The protocol notifies the programs of changes to times */
                Pause = 0x08, /**< The protocol supports pausing the clock */
                History = 0x10,
                Undo = 0x20, /**< It is possible to undo a move */
                GameOver = 0x40 /**< The protocol emits gameOver() when the game is over */
            };
            Q_DECLARE_FLAGS ( Features, Feature )

            enum ErrorCode
            {
                NoError = 0,
                UserCancelled,
                NetworkError,
                InstallationError,
                UnknownError
            };

            static QString stringFromErrorCode ( ErrorCode code );

            Protocol ( QObject* parent = 0 );
            virtual ~Protocol();

            // Needed functions

            Color playerColor() const;
            QString opponentName() const;
            QString playerName() const;
            QVariant attribute ( const QString& attribute ) const;

        protected:
            void setPlayerColor ( Color color );
            void setOpponentName ( const QString& name );
            void setPlayerName ( const QString& name );
            void setAttribute ( const QString& attribute, QVariant value );
            void setAttributes ( QVariantMap attributes );

        public Q_SLOTS:
            virtual void move ( const Move& m ) = 0;
            virtual void startGame() = 0;
            virtual void init ( const QVariantMap& options ) = 0;

            // Optional features
        public:
            virtual Features supportedFeatures();
            virtual Move::List moveHistory();
            int timeRemaining();

        public Q_SLOTS:
            virtual void pauseGame();
            virtual void resumeGame();
            virtual void undoLastMove();
            virtual void setOpponentTimeLimit ( int seconds );
            virtual void setPlayerTimeLimit ( int seconds );

        Q_SIGNALS:
            void pieceMoved ( const Move& m );
            void illegalMove();
            void gameOver ( const Color& winner );

            void errorStringChanged ( const QString& errorString );
            void errorCodeChanged ( const ErrorCode& error );

            void initSuccesful();
            void error ( const Protocol::ErrorCode& errorCode, const QString& errorString = QString() );

            void timeChanged ( const Color& color, const QTime& time );

        private:
            ProtocolPrivate* d_ptr;
            Q_DECLARE_PRIVATE ( Protocol )
    };

    Q_DECLARE_OPERATORS_FOR_FLAGS ( Protocol::Features )
}

#endif // KNIGHTS_PROTOCOL_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;
