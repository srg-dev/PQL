#ifndef __HELPH
#define __HELPH

#define HELPW	650
#define HELPH	700
#define HELPIND1	15
#define HELPIND2	30
#define HELPIND3	45
#define HELPIND4	60
#define HELPIND5	75
#define HELPBFONT	"Sans 11"
#define HELPBBFONT	"Sans bold 11"
#define HELPHFONT1	"Sans bold 15"
#define HELPHFONT2	"Sans bold 13"

enum {
	H1,		// HEADER 1
	H2,		// HEADER 2
	BN0,		// normal body, 0 after
	BB0,		// bold body, 0 after
	BN1,		// normal body, 1 line after
	BB1		// bold body, 1 line after
};

enum {
	INTROHELP,
	TRACEHELP,
	MAGHELP,
	SPECHELP,
	SPLITHELP,
	HEADHELP,
	CTRLSHELP
};
#endif
