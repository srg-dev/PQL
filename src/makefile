EXEC	= ${PQLBIN}/pql

PKGCFGC=`pkg-config --cflags gtk+-2.0`
PKGCFGL=`pkg-config --libs gtk+-2.0 gthread-2.0`

CFLAGS= $(FLOAT) $(INCLUDE) -D$(ARCH)  $(ADD_CFLAGS) $(DEBUG) -DPQL_ONLY $(PKGCFGC) -I$(PQLINC)
LFLAGS= $(PQLLFLAGS) $(PKGCFGL)
LIBS = $(PQLLIB)/libdata.a $(PQLLIB)/libmseed.a $(PQLLIB)/libvfbb.a $(PQLLIB)/libutils.a \
		$(PQLLIB)/libtform.a $(PQLLIB)/libdbl.a $(PQLLIB)/libtdb.a

HEADS=	$(PQLINC)/pql_externs.h \
	$(PQLINC)/gtk_pql_externs.h \
	$(PQLINC)/pql_defines.h  \
	$(PQLINC)/pql_data_structs.h

SRCS=	globals.c \
	gtk_pql.c \
	gtk_controls.c \
	gtk_help.c \
	gtk_events.c \
	gtk_data.c \
	gtk_globals.c \
	gtk_filters.c \
	gtk_filters_cmn.c \
	gtk_file_select.c \
	gtk_draw.c \
	gtk_util.c \
	gtk_mouse.c \
	gtk_drag.c \
	amps.c \
	data.c \
	thread_file.c \
	thread_trace.c \
   	iir.c \
	fftsubs.c \
	plot_trace.c  \
	plot_mag.c  \
	plot_spectra.c \
	plot_extras.c  \
	printPS.c \
	printPNG.c \
	defaults.c \
	trace.c \
	magnify.c \
	transform.c \
	axes.c \
	sort.c	\
	sort_controls.c \
	pql.c \
	xAxis.c \
	agc_py.c

OBJS =	$(SRCS:%.c=$(ARCH)/%.o) 

all:    $(EXEC)

$(EXEC):: $(OBJS) $(LIBS)
	$(CC) -o $(EXEC) $(CFLAGS) $(OBJS) $(LFLAGS) $(LIBS) -ltirpc -lnsl 

$(ARCH)/%.o: %.c $(HEADS)
		$(CC) $(CFLAGS) -c $(@F:.o=.c) -o $@

clean:
	@$(RM) $(ARCH)/*.o $(EXEC) *~ *.o

