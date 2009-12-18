# Generic makefile - Unix (rs6000) version
#
            LDLIBS = -lXm -lXt -lXmu -lX11 -lMrm
         LDOPTIONS = 
            CFLAGS = -O2 $(DEFINES) 
                RM = rm -f
               UIL = uil

include CommonMakefile

$(PGM):: $(OBJS) 
	$(RM) $@
	$(CC) -o $@ $(OBJS) $(LDOPTIONS) $(LDLIBS)

install:
	/usr/bin/install -c -s -m 755 midaswww /usr/local/bin
	/usr/bin/install -c -m 644 midas.uid   /usr/local/lib
	/usr/bin/install -c -m 644 midaswww.uid  /usr/local/lib

clean:
	rm -f *.o midaswww *.uid *.uic *.adh

