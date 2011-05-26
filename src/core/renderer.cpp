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

#include "renderer.h"

using namespace Knights;

#if defined WITH_KGR 

Renderer::Renderer ( const QString& defaultTheme ) : KGameRenderer ( QLatin1String("themes/default.desktop") )
{
    setTheme ( defaultTheme );
}
Renderer::~Renderer () 
{

}

#else
#include <KGameTheme>

Renderer::Renderer ( const QString& defaultTheme )
{
    m_theme = new KGameTheme;
    setTheme ( defaultTheme );
}

Renderer::~Renderer()
{
    delete m_theme;
}

bool Renderer::spriteExists ( const QString& key )
{
    return elementExists ( key );
}

QRectF Renderer::boundsOnSprite ( const QString& key )
{
    return boundsOnElement ( key );
}

void Renderer::setTheme ( const QString& theme )
{
    if ( !m_theme->load ( theme ) )
    {
        m_theme->loadDefault();
    }
    load ( m_theme->graphics() );
}

#endif // WITH_KGR
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;
