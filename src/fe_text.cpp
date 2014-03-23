/*
 *
 *  Attract-Mode frontend
 *  Copyright (C) 2013 Andrew Mickelson
 *
 *  This file is part of Attract-Mode.
 *
 *  Attract-Mode is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Attract-Mode is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Attract-Mode.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "fe_text.hpp"
#include "fe_settings.hpp"
#include "fe_util.hpp"
#include "fe_shader.hpp"
#include <iostream>

FeText::FeText( const std::string &str, int x, int y, int w, int h )
	: FeBasePresentable( false ),
	m_string( str ),
	m_index_offset( 0 ),
	m_user_charsize( -1 ),
	m_size( w, h ),
	m_position( x, y )
{
}

void FeText::setFont( const sf::Font &f )
{
	m_draw_text.setFont( f );
}

const sf::Vector2f &FeText::getPosition() const
{
	return m_position;
}

void FeText::setPosition( const sf::Vector2f &p )
{
	m_position = p;
	script_do_update( this );
}

const sf::Vector2f &FeText::getSize() const
{
	return m_size;
}

void FeText::setSize( const sf::Vector2f &s )
{
	m_size = s;
	script_do_update( this );
}

float FeText::getRotation() const
{
	return m_draw_text.getRotation();
}

void FeText::setRotation( float r )
{
	m_draw_text.setRotation( r );
	script_flag_redraw();
}

void FeText::setColor( const sf::Color &c )
{
	m_draw_text.setColor( c );
	script_flag_redraw();
}

const sf::Color &FeText::getColor() const
{
	return m_draw_text.getColor();
}

void FeText::setIndexOffset( int io )
{
	if ( m_index_offset != io )
	{
		m_index_offset=io;
		script_do_update( this );
	}
}

int FeText::getIndexOffset() const
{
	return m_index_offset;
}

void FeText::on_new_list( FeSettings *s, float scale_x, float scale_y )
{
	//
	// Apply the scale factors using m_base_text to get m_draw_text
	//
	sf::Transform scaler;
	scaler.scale( scale_x, scale_y );

	sf::Vector2f pos = scaler.transformPoint( getPosition() );
	sf::Vector2f size = scaler.transformPoint( getSize() );

	int char_size = 10;
	if ( m_user_charsize > 0 )
		char_size = m_user_charsize * scale_y;
	else if ( size.y > 14 )
		char_size = size.y - 4;

	m_draw_text.setPosition( pos );
	m_draw_text.setSize( size );
	m_draw_text.setCharacterSize( char_size );
}

void FeText::on_new_selection( FeSettings *feSettings )
{
	//
	// Perform substitutions of the [XXX] sequences occurring in m_string
	//
	size_t n = std::count( m_string.begin(), m_string.end(), '[' );

	std::string str = m_string;
	for ( int i=0; ((i< FeRomInfo::LAST_INDEX) && ( n > 0 )); i++ )
	{
		if ( i == FeRomInfo::Title ) // this is a special case dealt with below
			continue;

		std::string from = "[";
		from += FeRomInfo::indexStrings[i];
		from += "]";

		n -= perform_substitution( str, from,
				feSettings->get_rom_info( m_index_offset, (FeRomInfo::Index)i) );
	}

	if ( n > 0 )
	{
		n -= perform_substitution( str, "[ListTitle]",
				feSettings->get_current_list_title() );

		n -= perform_substitution( str, "[ListFilterName]",
				feSettings->get_current_filter_name() );

		n -= perform_substitution( str, "[ListSize]",
				as_str( feSettings->get_current_list_size() ) );

		n -= perform_substitution( str, "[ListEntry]",
				as_str( feSettings->get_rom_index() + 1 ) );
	}

	if ( n > 0 )
	{
		const std::string &title_full =
				feSettings->get_rom_info( m_index_offset, FeRomInfo::Title );

		n -= perform_substitution( str, "[TitleFull]", title_full );

		if ( feSettings->hide_brackets() )
		{
			std::string title;
			size_t pos = title_full.find_first_of( "([" );

			if ( pos == std::string::npos )
				title = title_full;
			else
			{
				title = title_full.substr( 0,
						title_full.find_last_of( FE_WHITESPACE, pos ) );
			}

			n -= perform_substitution( str, "[Title]", title );
		}
		else
			n -= perform_substitution( str, "[Title]", title_full );
	}

	m_draw_text.setString( str );
}

void FeText::draw( sf::RenderTarget &target, sf::RenderStates states ) const
{
	FeShader *s = get_shader();
	if ( s )
	{
		const sf::Shader *sh = s->get_shader();
		if ( sh )
			states.shader = sh;
	}

	target.draw( m_draw_text, states );
}

void FeText::set_word_wrap( bool w )
{
	m_draw_text.setWordWrap( w );
}

bool FeText::get_word_wrap()
{
	return m_draw_text.getWordWrap();
}

const char *FeText::get_string()
{
	return m_string.c_str();
}

void FeText::set_string(const char *s)
{
	m_string=s;
	script_do_update( this );
}

int FeText::get_bgr()
{
	return m_draw_text.getBgColor().r;
}

int FeText::get_bgg()
{
	return m_draw_text.getBgColor().g;
}

int FeText::get_bgb()
{
	return m_draw_text.getBgColor().b;
}

int FeText::get_bga()
{
	return m_draw_text.getBgColor().a;
}

int FeText::get_charsize()
{
	return m_draw_text.getCharacterSize();
}

int FeText::get_style()
{
	return m_draw_text.getStyle();
}

int FeText::get_align()
{
	return (int)m_draw_text.getAlignment();
}

const char *FeText::get_font()
{
	return m_font_name.c_str();
}

void FeText::set_bgr(int r)
{
	sf::Color c=m_draw_text.getBgColor();
	c.r=r;
	m_draw_text.setBgColor(c);
	script_flag_redraw();
}

void FeText::set_bgg(int g)
{
	sf::Color c=m_draw_text.getBgColor();
	c.g=g;
	m_draw_text.setBgColor(c);
	script_flag_redraw();
}

void FeText::set_bgb(int b)
{
	sf::Color c=m_draw_text.getBgColor();
	c.b=b;
	m_draw_text.setBgColor(c);
	script_flag_redraw();
}

void FeText::set_bga(int a)
{
	sf::Color c=m_draw_text.getBgColor();
	c.a=a;
	m_draw_text.setBgColor(c);
	script_flag_redraw();
}

void FeText::set_bg_rgb(int r, int g, int b )
{
	sf::Color c=m_draw_text.getBgColor();
	c.r=r;
	c.g=g;
	c.b=b;
	m_draw_text.setBgColor(c);
	script_flag_redraw();
}

void FeText::set_charsize(int s)
{
	m_user_charsize = s;
	script_do_update( this );
}

void FeText::set_style(int s)
{
	m_draw_text.setStyle(s);
	script_flag_redraw();
}

void FeText::set_align(int a)
{
	m_draw_text.setAlignment( (FeTextPrimative::Alignment)a);
	script_flag_redraw();
}

void FeText::set_font( const char *f )
{
	const sf::Font *font = script_get_font( f );

	if ( font )
	{
		setFont( *font );
		m_font_name = f;

		script_flag_redraw();
	}
}