CFLAGS = `pkg-config --cflags gtk+-2.0`
LDFLAGS = -lm `pkg-config --libs gtk+-2.0`

ags_test_dial: ags_test_dial.o ags_dial.o
	gcc -g -o ags_test_dial ags_test_dial.o ags_dial.o $(LDFLAGS)

ags_test_dial.o: ags/test/ags_test_dial.[ch]
	gcc -g -c -o ags_test_dial.o ags/test/ags_test_dial.c  $(CFLAGS) $(LDFLAGS)

ags_dial.o: ags/widget/ags_dial.[ch]
	gcc -g -c -o ags_dial.o ags/widget/ags_dial.c $(CFLAGS) $(LDFLAGS)
