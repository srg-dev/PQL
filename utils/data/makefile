LIB=libdata.a
PKG=`pkg-config --cflags gtk+-2.0 cairo`

CFLAGS= $(PQLCFLAGS) $(FLOAT) $(PKG) -D$(ARCH) $(DEBUG) -I$(PQLINC) -Iinclude -I../../contrib/vfbb/src -DPQL_ONLY

HEADS=	$(PQLINC)/pql_externs.h \
		$(PQLINC)/pql_data_structs.h

SRCS= data_ah.c \
		data_ahio.c \
		data_convert.c \
		data_dr100.c \
		data_endian.c \
		data_mseed.c \
		data_nano.c \
		data_readhead.c \
		data_sac.c \
		data_segy.c \
		data_x.c \
		data_entry.c \
		data_rt130.c \
		data_rt125a.c \
		data_rt125.c \
		reftek_utils.c \
		steim.c \
		steim2.c \
		data_utils.c \
		data_excerpt.c

OBJS=	$(SRCS:%.c=$(ARCH)/%.o) 

all:    $(LIB)

$(LIB):: $(OBJS)
	ar cr $(ARCH)/$(LIB) $(OBJS)
	cp $(ARCH)/$(LIB) $(PQLLIB)
	ranlib $(PQLLIB)/$(LIB)

#$(OBJS): %.c $(HEADS)
$(ARCH)/%.o:	%.c $(HEADS)
		sh -c "$(CC) $(CFLAGS) -c $(@F:.o=.c) -o $@"

clean:
	@$(RM) $(ARCH)/*.o $(ARCH)/$(LIB) *~ *.o
