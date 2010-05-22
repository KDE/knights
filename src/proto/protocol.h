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
    class Protocol : public QObject
    {
            Q_OBJECT
            Q_ENUMS ( Feature )
            Q_ENUMS ( ErrorCode )
            Q_FLAGS ( Features )
            Q_PROPERTY ( Knights::Piece::Color playerColor READ playerColor WRITE setPlayerColor NOTIFY playerColorChanged )

        public:
            enum Feature
            {
                NoFeatures = 0,
                TimeLimit = 1,
                ChangeTimeLimit = 2,
                Pause = 4,
                History = 8,
                Undo = 16
            };
            Q_DECLARE_FLAGS(Features, Feature)

            enum ErrorCode
            {
                NoError = 0,
                UserCancelled,
                NetworkError,
                InstallationError,
                UnknownError
            };

            static QString stringFromErrorCode ( ErrorCode code );

            Protocol ( QObject* parent = 0 ) : QObject ( parent ) {};
            virtual ~Protocol() {};

            // Needed functions

            Piece::Color playerColor() const;

        protected:
            void setPlayerColor ( Piece::Color color );

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
            void gameOver ( Piece::Color winner );

            void colorChanged ( Piece::Color playerColor );
            void errorStringChanged ( const QString& errorString );
            void errorCodeChanged ( ErrorCode error );

            void initSuccesful();
            void error ( Protocol::ErrorCode errorCode, const QString& errorString = QString() );

        private:
            Piece::Color m_color;
    };

    Q_DECLARE_OPERATORS_FOR_FLAGS ( Protocol::Features )
}

#endif // KNIGHTS_PROTOCOL_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;
