interpreter: srce.o scan.o parser.o table.o report.o stkmc.o cgen.o cln.o
	g++ srce.o scan.o parser.o table.o report.o stkmc.o cgen.o cln.o -o interpreter -lm
srce.o: srce.cpp srce.h misc.h
	g++ -c srce.cpp
scan.o: scan.cpp scan.h srce.h report.h misc.h
	g++ -c scan.cpp
parser.o: parser.cpp parser.h set.h srce.h scan.h table.h report.h misc.h cgen.h
	g++ -c parser.cpp
table.o: table.cpp table.h misc.h report.h cgen.h
	g++ -c table.cpp
report.o: report.cpp report.h misc.h
	g++ -c report.cpp
stkmc.o: stkmc.cpp stkmc.h misc.h
	g++ -c stkmc.cpp
cgen.o: cgen.cpp cgen.h report.h stkmc.h misc.h
	g++ -c cgen.cpp
cln.o: cln.cpp misc.h srce.h scan.h parser.h table.h report.h stkmc.h cgen.h
	g++ -c cln.cpp
clean:
	rm -f interpreter misc.o srce.o scan.o parser.o table.o report.o stkmc.o cgen.o
install:
	cp interpreter /usr/local/bin/interpreter
uninstall:
	rm -f /usr/local/bin/interpreter