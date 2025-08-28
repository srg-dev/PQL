LIB=libutils.a
INCLUDES=-I$(PQLINC)

CFLAGS= `pkg-config --cflags gtk+-2.0 cairo` $(FLOAT) -D$(ARCH) $(DEBUG) $(INCLUDES) -DPQL_ONLY

HEADS=	$(PQLINC)/pql_data_structs.h \
		$(PQLINC)/db_iface.h

SRCS=	util_time.c \
		plotTForm.c \
		threads.c

OBJS=	$(SRCS:%.c=$(ARCH)/%.o) 

all:    $(LIB)

$(LIB):: $(OBJS)
	ar cr $(ARCH)/$(LIB) $(OBJS)
	cp $(ARCH)/$(LIB) $(PQLLIB)
	ranlib $(PQLLIB)/$(LIB)

#$(OBJS): %.c $(HEADS)
$(ARCH)/%.o:	%.c $(HEADS)
		$(CC) $(CFLAGS) -c $(@F:.o=.c) -o $@

clean:
	@$(RM) $(ARCH)/*.o $(ARCH)/$(LIB) *~ *.o
