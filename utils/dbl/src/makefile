LIB=libdbl.a

CFLAGS= $(PQLCFLAGS) -I$(MYSQLINC) $(FLOAT) -D$(ARCH) $(DEBUG) -I$(PQLINC) `pkg-config --cflags glib-2.0` -DPQL_ONLY

HEADS=	$(PQLINC)/dbl_iface.h

SRCS=	dbl_entry.c \
		dbl_utils_tdb.c	\
		dbl_defaults.c \
		dbl_filters.c

#		dbl_pdfsrvrs.c \
#		dbl_STN.c
#		dbl_pdf.c

OBJS=	$(SRCS:%.c=$(ARCH)/%.o) 

all:    $(LIB)

$(LIB):: $(OBJS)
	ar cr $(ARCH)/$(LIB) $(OBJS)
	cp $(ARCH)/$(LIB) $(PQLLIB)
	ranlib $(PQLLIB)/$(LIB)

$(ARCH)/%.o:	%.c $(HEADS)
		sh -c "$(CC) $(CFLAGS) -c $(@F:.o=.c) -o $@"

clean:
	@$(RM) $(ARCH)/*.o $(LIB) *~ *.o
