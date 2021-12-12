rccpath = "C:\Qt\6.2.1\msvc2019_64\bin\rcc.exe"

indigo.rcc: indigo.qrc
	$(rccpath) -binary indigo.qrc -o indigo.rcc

clean:
	rm indigo.rcc

remake: clean indigo.rcc
