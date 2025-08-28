// SPECTRA HELP
#define SPECH1 "SPECTRA Screen\n"
#define SPECB1 "The SPECTRA screen displays the FFT (fast fourier transform) of the selected portion of the selected traces on the TRACE screen (also exactly corresponding to the selected portion currently displayed in the MAGNIFY screen).  Prior to transfrom, the selection is demeaned and tapered with a 10% cosine filter.  The FFTs are presented with the:\n"
#define SPECB1A "AMPLITUDE Axis to the left of the transforms, and\n"
#define SPECB1B "PERIOD Axis at the top of the transforms\n"
#define SPECB1C "FREQUENCY Axis at the bottom of the transforms\n"

#define SPECH2 "SIDEBAR Functions\n"
#define SPECH2A "DISPLAY Settings\n"

#define SPECB2	"Window Scale:\n"
#define SPECB2A	"Define the Y-Axis maximum and minimum values of each individual Transform\n"
#define SPECB21	"Trace\n"
#define SPECB211	"Use max and min values of the Transform itself\n"
#define SPECB22	"Window:\n"
#define SPECB221	"Use max and min values of all Transforms currently displayed\n"

#define SPECB3	"Display as:\n"
#define SPECB3A	"COUNTS or VOLTS, see TRACE Help for details\n"

#define SPECB4	"Overlay:\n"
#define SPECB4A	"Turn Overlay mode ON and OFF.  Overlay colors defined in Controls Panel.\n"

#define SPECB5	"Filter:\n"
#define SPECB5A	"Turn Filter ON and OFF.  Filter settings defined in Controls Panel.  See CONTROLS Help for more info.\n"

#define SPECH3 "MOUSE Functions\n"
#define SPECB6 "Specifying Filter Cutoff Frequency\n"
#define SPECB6A "Two methods exist in PQL for defining the filter cutoff frequency, explicitly providing this cutoff via the Controls Panel Filter Settings (see CONTROLS Panel Help for details on Filter Settings), or via the SPECTRA screen, described here.\nWithin the plot region, one or two cutoff frequencies can be specified (CLICK = make vertical line at desired cutoff frequency) and will define a new filter cutoff frequency according to the following rules:\n"
#define SPECB61 "If a single cutoff frequency is specified, this new cutoff will be applied to whichever filter is currently turned on.\n"
#define SPECB62 "If both HIGH and LOW Pass filters are ON (or OFF), the new cutoff will become the LOW Pass filter cutoff if greater than the current cutoff frequency, otherwise the new cutoff will become the HIGH Pass filter cutoff.\n"
#define SPECB63 "If two cutoff frequencies are specified, the higher frequency cutoff becomes the new LOW Pass filter cutoff and the lesser frequency becomes the new HIGH Pass filter cutoff.\n"

#define SPECB7 "Applying New Cutoffs/Re-Executing Transformation\n"
#define SPECB7A "To set the new cutoff frequencies for the filters, SHIFT+CLICK anywhere within the plot region.\nIf either filter is currently ON (Poles <> 0 and Sidebar Filter Setting = ON), the transformation will automatically be re-executed and displayed.\n"

#define SPECB8 "Display Data Point Values - Frequency and Amplitude\n"
#define SPECB8A "CTRL+CLICK within desired transform, information is displayed in status bar at bottom.\n"
#define SPECB9 "Scan Data Point Values - Frequency and Amplitude\n"
#define SPECB9A "CTRL+CLICK+DRAG within desired transform.\n"

#define SPECB10	"Bring Transform to TOP\n"
#define SPECB10A "In OVERLAY mode, Transforms may be obscured by others plotted on top.  To re-plot a Transform on top, CLICK on the Transform label.  Defining this, then, to be the current 'Top Transform' also specifies that Data Point Display functionality will also act on this Transform.  Default 'Top Transform' is the last Transform on display.\n" 

#define SPECH4 "CONTROL Screen Options\n"
#define SPECB11 "Amplitude:\n"
#define SPECB11A "Define the display settings for the Transformation Amplitude Axis\n"
#define SPECB111 "Scale:\n"
#define SPECB1111 "Define Amplitude Axis to be displayed as either Logarithmic or Linear.\n"
#define SPECB112 "Maximum:\n"
#define SPECB1121 "Define the Maximum Amplitude to display, this is an absolute value.\n"
#define SPECB113 "Decades:\n"
#define SPECB1131 "Define the Maximum number of Decades to display\n"

#define SPECB12 "Frequency:\n"
#define SPECB12A "Define the display settings for the Transformation Frequency Axis\n"
#define SPECB121 "Scale:\n"
#define SPECB1211 "Define Frequency Axis to be displayed as either Logarithmic or Linear.\n"
#define SPECB122 "Maximum:\n"
#define SPECB1221 "Define the Maximum Frequency for display, this is an absolute value.\n"
#define SPECB123 "Minimum:\n"
#define SPECB1231 "Define the Minimum Frequency for display, this is an absolute value.\n"

#define SPECB13 "Colors\n"
#define SPECB13A "Define the various settable colors for the screen\n"
#define SPECB14 "Buttons\n"
#define SPECB141 "Continue\n"
#define SPECB141A "Apply new settings and return to the SPECTRA screen\n"

