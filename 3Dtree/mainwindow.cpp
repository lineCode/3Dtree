
/*!
	\file

	\author Igor Mironchik (igor.mironchik at gmail dot com).

	Copyright (c) 2017 Igor Mironchik

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// 3Dtree include.
#include "mainwindow.hpp"
#include "branch.hpp"
#include "constants.hpp"

#include "leaf.hpp"

// Qt include.
#include <QPushButton>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QLabel>
#include <QTimer>

#include <Qt3DCore/QEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QPointLight>
#include <Qt3DCore/QTransform>


//! Grow timer in milliseconds.
static const int c_growTimer = 1000;

//
// MainWindowPrivate
//

class MainWindowPrivate {
public:
	MainWindowPrivate( MainWindow * parent )
		:	m_tree( Q_NULLPTR )
		,	m_growSpeed( 1.0f / 60.0f / ( 1000.0f / (float) c_growTimer ) )
		,	m_currentAge( 0.0f )
		,	m_startPos( 0.0f, -0.5f, 0.0f )
		,	m_endPos( 0.0f, 0.0f, 0.0f )
		,	m_years( Q_NULLPTR )
		,	m_btn( Q_NULLPTR )
		,	m_timer( Q_NULLPTR )
		,	m_playing( true )
		,	m_grown( false )
		,	m_rootEntity( Q_NULLPTR )
		,	q( parent )
	{
	}

	//! Init.
	void init( Qt3DExtras::Qt3DWindow * view );
	//! Init 3D.
	void init3D( Qt3DExtras::Qt3DWindow * view );
	//! Create tree.
	void createTree();

	//! Tree.
	Branch * m_tree;
	//! Grow speed.
	float m_growSpeed;
	//! Current age.
	float m_currentAge;
	//! Start tree pos.
	QVector3D m_startPos;
	//! End tree pos.
	QVector3D m_endPos;
	//! Years.
	QSpinBox * m_years;
	//! Pause/play button.
	QPushButton * m_btn;
	//! Timer.
	QTimer * m_timer;
	//! Playing?
	bool m_playing;
	//! Tree grown.
	bool m_grown;
	//! Root entity.
	Qt3DCore::QEntity * m_rootEntity;
	//! Parent.
	MainWindow * q;
}; // class MainWindowPrivate

void
MainWindowPrivate::init( Qt3DExtras::Qt3DWindow * view )
{
	QWidget * container = QWidget::createWindowContainer( view, q );
	container->setMinimumSize( QSize( 768, 600 ) );
	container->setMaximumSize( QSize( 768, 600 ) );

	QHBoxLayout * l = new QHBoxLayout( q );
	l->addWidget( container );

	QVBoxLayout * v = new QVBoxLayout;
	l->addLayout( v );

	QHBoxLayout * l1 = new QHBoxLayout;
	v->addLayout( l1 );

	QLabel * label = new QLabel( MainWindow::tr( "Maximum Years" ), q );
	l1->addWidget( label );

	m_years = new QSpinBox( q );
	m_years->setMinimum( 1 );
	m_years->setMaximum( 99 );
	m_years->setValue( 3 );
	l1->addWidget( m_years );

	m_btn = new QPushButton( MainWindow::tr( "Pause" ), q );
	v->addWidget( m_btn );

	QSpacerItem * s = new QSpacerItem( 10, 10, QSizePolicy::Minimum,
		QSizePolicy::Expanding );

	v->addItem( s );

	m_timer = new QTimer( q );
	m_timer->setInterval( c_growTimer );
	m_timer->start();

	MainWindow::connect( m_btn, &QPushButton::clicked,
		q, &MainWindow::buttonClicked );
	MainWindow::connect( m_timer, &QTimer::timeout,
		q, &MainWindow::timer );

	init3D( view );
}

void MainWindowPrivate::init3D( Qt3DExtras::Qt3DWindow * view )
{
	// Root entity
	m_rootEntity = new Qt3DCore::QEntity;

	// Camera
	Qt3DRender::QCamera * cameraEntity = view->camera();

	cameraEntity->lens()->setPerspectiveProjection(
		45.0f, 16.0f / 9.0f, 0.1f, 1000.0f );
	cameraEntity->setPosition( QVector3D( 0.0f, 0.0f, 20.0f ) );
	cameraEntity->setUpVector( QVector3D( 0.0f, 1.0f, 0.0f ) );
	cameraEntity->setViewCenter( QVector3D( 0.0f, 0.0f, 0.0f ) );

	Qt3DCore::QEntity * lightEntity = new Qt3DCore::QEntity( m_rootEntity );

	Qt3DRender::QPointLight * light = new Qt3DRender::QPointLight( lightEntity );
	light->setColor( Qt::white );
	light->setIntensity( 1.0f );
	lightEntity->addComponent( light );

	Qt3DCore::QTransform * lightTransform = new Qt3DCore::QTransform(
		lightEntity );
	lightTransform->setTranslation( cameraEntity->position() );
	lightEntity->addComponent( lightTransform );

	createTree();

	view->setRootEntity( m_rootEntity );
}

void
MainWindowPrivate::createTree()
{
	if( m_tree )
		m_tree->deleteLater();

	m_tree = new Branch( m_startPos, m_endPos, c_startBranchRadius,
		true, m_rootEntity );

	m_tree->setAge( 0.0f );
}


//
// MainWindow
//

MainWindow::MainWindow( Qt3DExtras::Qt3DWindow * view )
	:	d( new MainWindowPrivate( this ) )
{
	d->init( view );
}

MainWindow::~MainWindow()
{
}

void
MainWindow::buttonClicked()
{
	if( d->m_grown )
	{
		d->m_playing = true;

		d->m_btn->setText( tr( "Pause" ) );

		d->m_currentAge = 0.0f;

		d->createTree();

		d->m_timer->start();
	}
	else if( d->m_playing )
	{
		d->m_playing = false;

		d->m_btn->setText( tr( "Play" ) );

		d->m_timer->stop();
	}
	else
	{
		d->m_playing = true;

		d->m_btn->setText( tr( "Pause" ) );

		d->m_timer->start();
	}
}

void
MainWindow::timer()
{
	d->m_currentAge += d->m_growSpeed;

	if( d->m_currentAge > (float) d->m_years->value() - 0.5f )
	{
		d->m_timer->stop();

		d->m_grown = true;

		d->m_btn->setText( tr( "Restart" ) );

		d->m_playing = false;
	}
	else
		d->m_tree->setAge( d->m_currentAge );
}
