/* === This file is part of Tomahawk Player - <http://tomahawk-player.org> ===
 *
 *   Copyright 2010-2011, Christian Muehlhaeuser <muesli@tomahawk-player.org>
 *
 *   Tomahawk is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Tomahawk is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Tomahawk. If not, see <http://www.gnu.org/licenses/>.
 */

#include "PlaylistChartItemDelegate.h"

#include <QApplication>
#include <QPainter>

#include "query.h"
#include "result.h"
#include "artist.h"
#include "source.h"
#include "sourcelist.h"

#include "trackmodel.h"
#include "trackmodelitem.h"
#include "trackproxymodel.h"
#include "trackview.h"
#include "trackheader.h"

#include "utils/tomahawkutilsgui.h"
#include "utils/logger.h"

using namespace Tomahawk;


PlaylistChartItemDelegate::PlaylistChartItemDelegate( TrackView* parent, TrackProxyModel* proxy )
    : QStyledItemDelegate( (QObject*)parent )
    , m_view( parent )
    , m_model( proxy )
{
    m_topOption = QTextOption( Qt::AlignTop );
    m_topOption.setWrapMode( QTextOption::NoWrap );

    m_centerOption = QTextOption( Qt::AlignCenter );
    m_centerOption.setWrapMode( QTextOption::NoWrap );

    m_centerRightOption = QTextOption( Qt::AlignVCenter | Qt::AlignRight );
    m_centerRightOption.setWrapMode( QTextOption::NoWrap );

    m_bottomOption = QTextOption( Qt::AlignBottom );
    m_bottomOption.setWrapMode( QTextOption::NoWrap );
}


QSize
PlaylistChartItemDelegate::sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    QSize size = QStyledItemDelegate::sizeHint( option, index );

    int stretch = 0;
    switch ( index.row() )
    {
        case 0:
            stretch = 6;
            break;
        case 1:
            stretch = 5;
            break;
        case 2:
            stretch = 4;
            break;
            
        default:
            if ( index.row() < 10 )
                stretch = 3;
            else
                stretch = 2;
    }

    if ( index.isValid() )
    {
        int rowHeight = option.fontMetrics.height() + 8;
        size.setHeight( rowHeight * stretch );
    }

    return size;
}


QWidget*
PlaylistChartItemDelegate::createEditor( QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    Q_UNUSED( parent );
    Q_UNUSED( option );
    Q_UNUSED( index );
    return 0;
}


void
PlaylistChartItemDelegate::prepareStyleOption( QStyleOptionViewItemV4* option, const QModelIndex& index, TrackModelItem* item ) const
{
    initStyleOption( option, index );

    if ( item->isPlaying() )
    {
        option->palette.setColor( QPalette::Highlight, option->palette.color( QPalette::Mid ) );
        option->state |= QStyle::State_Selected;
    }

    if ( option->state & QStyle::State_Selected )
    {
        option->palette.setColor( QPalette::Text, option->palette.color( QPalette::HighlightedText ) );
    }
    else
    {
        float opacity = 0.0;
        if ( item->query()->results().count() )
            opacity = item->query()->results().first()->score();

        opacity = qMax( (float)0.3, opacity );
        QColor textColor = TomahawkUtils::alphaBlend( option->palette.color( QPalette::Text ), option->palette.color( QPalette::BrightText ), opacity );

        option->palette.setColor( QPalette::Text, textColor );
    }
}


void
PlaylistChartItemDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    TrackModelItem* item = m_model->itemFromIndex( m_model->mapToSource( index ) );
    Q_ASSERT( item );

    QStyleOptionViewItemV4 opt = option;
    prepareStyleOption( &opt, index, item );
    opt.text.clear();

    qApp->style()->drawControl( QStyle::CE_ItemViewItem, &opt, painter );

    if ( m_view->header()->visualIndex( index.column() ) > 0 )
        return;

    QPixmap pixmap, avatar;
    QString artist, track, upperText, lowerText;
    unsigned int duration = 0;

    if ( item->query()->results().count() )
    {
        artist = item->query()->results().first()->artist()->name();
        track = item->query()->results().first()->track();
        duration = item->query()->results().first()->duration();
    }
    else
    {
        artist = item->query()->artist();
        track = item->query()->track();
    }

    painter->save();
    {
        QRect r = opt.rect.adjusted( 3, 6, 0, -6 );

        // Paint Now Playing Speaker Icon
        if ( item->isPlaying() )
        {
            QPixmap nowPlayingIcon = TomahawkUtils::defaultPixmap( TomahawkUtils::NowPlayingSpeaker );
            QRect npr = r.adjusted( 3, r.height() / 2 - nowPlayingIcon.height() / 2, 18 - r.width(), -r.height() / 2 + nowPlayingIcon.height() / 2 );
            nowPlayingIcon = TomahawkUtils::defaultPixmap( TomahawkUtils::NowPlayingSpeaker, TomahawkUtils::Original, npr.size() );
            painter->drawPixmap( npr, nowPlayingIcon );
            r.adjust( 22, 0, 0, 0 );
        }

        QFont figureFont = opt.font;
        figureFont.setPixelSize( 18 );
        figureFont.setWeight( 99 );

        QFont boldFont = opt.font;
        boldFont.setPixelSize( 15 );
        boldFont.setWeight( 99 );

        QFont smallBoldFont = opt.font;
        smallBoldFont.setPixelSize( 12 );
        smallBoldFont.setWeight( 60 );

        QFont durationFont = opt.font;
        durationFont.setPixelSize( 12 );
        durationFont.setWeight( 80 );

        if ( index.row() == 0 )
        {
            figureFont.setPixelSize( 36 );
            boldFont.setPixelSize( 26 );
            smallBoldFont.setPixelSize( 22 );
        }
        else if ( index.row() == 1 )
        {
            figureFont.setPixelSize( 30 );
            boldFont.setPixelSize( 22 );
            smallBoldFont.setPixelSize( 18 );
        }
        else if ( index.row() == 2 )
        {
            figureFont.setPixelSize( 24 );
            boldFont.setPixelSize( 18 );
            smallBoldFont.setPixelSize( 14 );
        }
        else if ( index.row() >= 10 )
        {
            boldFont.setPixelSize( 12 );
            smallBoldFont.setPixelSize( 11 );
        }

        QRect figureRect = r.adjusted( 0, 0, -option.rect.width() + 60 - 6 + r.left(), 0 );
        painter->setFont( figureFont );
        painter->setPen( option.palette.text().color().lighter( 450 ) );
        painter->drawText( figureRect, QString::number( index.row() + 1 ), m_centerOption );
        painter->setPen( opt.palette.text().color() );

        QRect pixmapRect = r.adjusted( figureRect.width() + 6, 0, -option.rect.width() + figureRect.width() + option.rect.height() - 6 + r.left(), 0 );
        pixmap = item->query()->cover( pixmapRect.size(), false );
        if ( !pixmap )
        {
            pixmap = TomahawkUtils::defaultPixmap( TomahawkUtils::DefaultTrackImage, TomahawkUtils::ScaledCover, pixmapRect.size() );
        }
        painter->drawPixmap( pixmapRect, pixmap );
        
        r.adjust( pixmapRect.width() + figureRect.width() + 18, 1, -28, 0 );
        QRect leftRect = r.adjusted( 0, 0, -48, 0 );
        QRect rightRect = r.adjusted( r.width() - 40, 0, 0, 0 );

        painter->setFont( boldFont );
        QString text = painter->fontMetrics().elidedText( track, Qt::ElideRight, leftRect.width() );
        painter->drawText( leftRect, text, m_topOption );

        painter->setFont( smallBoldFont );
        text = painter->fontMetrics().elidedText( artist, Qt::ElideRight, leftRect.width() );
        painter->drawText( index.row() >= 10 ? leftRect : leftRect.adjusted( 0, painter->fontMetrics().height() + 6, 0, 0 ), text, index.row() >= 10 ? m_bottomOption : m_topOption );

        if ( duration > 0 )
        {
            painter->setFont( durationFont );
            text = painter->fontMetrics().elidedText( TomahawkUtils::timeToString( duration ), Qt::ElideRight, rightRect.width() );
            painter->drawText( rightRect, text, m_centerRightOption );
        }
    }
    painter->restore();
}
