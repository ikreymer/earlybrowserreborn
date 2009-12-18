# Generic makefile - Unix (rs6000) version
#
            LDLIBS = -lXm -lPW -lIM -lXt -lXmu -lX11 -lMrm -lbsd
         LDOPTIONS = -bnodelcsect
            CFLAGS = -O -DAIXV3 -D_BSD -Dqnoro $(DEFINES) 
                RM = rm -f
               UIL = uil

include CommonMakefile

$(PGM):: $(OBJS) 
	$(RM) $@
	$(CC) -o $@ $(OBJS) $(LDOPTIONS) $(LDLIBS)

install:
	/usr/ucb/install -c -s -m 755 midaswww /usr/local/bin
	/usr/ucb/install -c -m 644 midas.uid   /usr/local/lib
	/usr/ucb/install -c -m 644 midaswww.uid  /usr/local/lib

clean:
	rm -f *.o midaswww *.uid *.uic *.adh

