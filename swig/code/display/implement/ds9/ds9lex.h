#ifndef DISPLAY_DS9_DS9LEX_H_
#define DISPLAY_DS9_DS9LEX_H_

// Flex expects the signature of yylex to be defined in the macro YY_DECL, and
// the C++ parser expects it to be declared. We can factor both as follows.
#ifndef YY_DECL

#define YY_DECL                                     				\
    casa::viewer::ds9parse::token_type						\
    casa::viewer::ds9lex::lex( casa::viewer::ds9parse::semantic_type* yylval,	\
			       casa::viewer::ds9parse::location_type* yylloc	\
			     )
#endif

// including without guard results in yyFlexLexer being multiply defined when
// compiling the scanner...
#ifndef __FLEX_LEXER_H
#define yyFlexLexer ds9lex_FlexLexer
#include <FlexLexer.h>
#undef yyFlexLexer
#endif

#include <display/ds9/ds9context.h>
#include <display/ds9/ds9parse.hh>

namespace casa {
    namespace viewer {

	class Region;

	// Scanner is a derived class to add some extra function to the scanner
	// class. Flex itself creates a class named yyFlexLexer, which is renamed using
	// macros to ds9lex_FlexLexer. However we change the context of the generated
	// yylex() function to be contained within the ds9lex class. This is required
	// because the yylex() defined in ds9lex_FlexLexer has no parameters. */
	class ds9lex : public ds9lex_FlexLexer {
	    public:
		// Create a new scanner object. The streams arg_yyin and arg_yyout default
		// to cin and cout, but that assignment is only made when initializing in
		// yylex().
		ds9lex( std::istream* yyin=0, std::ostream* yyout=0 ) : ds9lex_FlexLexer( yyin, yyout ) { }
		~ds9lex( ) { }

		// This is the main lexing function. It is generated by flex according to
		// the macro declaration YY_DECL above. The generated bison parser then
		// calls this virtual function to fetch new tokens.
		virtual ds9parse::token_type lex( ds9parse::semantic_type* yylval, ds9parse::location_type* yylloc );

		void begin(int,int);

		// Enable debug output (via arg_yyout) if compiled into the scanner.
		void set_debug(bool b);

		void discard( int );
	};
    }
}


class CallBack {
    public:
	enum { SELECTCB, UNSELECTCB, HIGHLITECB, UNHIGHLITECB,
	       MOVEBEGINCB, MOVECB, MOVEENDCB, EDITBEGINCB,
	       EDITCB, EDITENDCB, ROTATEBEGINCB, ROTATECB,
	       ROTATEENDCB, DELETECB, TEXTCB, COLORCB,
	       LINEWIDTHCB, PROPERTYCB, FONTCB, KEYCB,
	       UPDATECB
	};
};

class Marker {
    public:
	// Select-- user may select the marker
	// Highlite-- user may highlite the marker
	// Edit-- user may edit the marker
	// Move-- user may move the marker
	// Rotate-- user may rotate the marker
	// Delete-- user may delete the marker
	// Fixed-- marker is fixed in size (not scaled based on zoom)
	// Include-- marker is 'included' or 'excluded' ie '+' or '-'
	// Source-- marker is a 'source' or 'background' marker
	// Dash-- render with dashed line
	enum Property { NONE=0, SELECT=1, HIGHLITE=2, EDIT=4, MOVE=8, ROTATE=16, 
			DELETE=32, FIXED=64, INCLUDE=128, SOURCE=256, DASH=512 };
};


typedef std::list<casa::viewer::Region*> bison_region_list;
#endif