TARGETS = xor or 0pad-blob

all: $(TARGETS)

.PHONY: clean all

clean:
	-rm $(TARGETS)

AUG_CFLAGS = $(CPPFLAGS) $(CFLAGS) -I .

.c:
	$(CC) $(AUG_CFLAGS) $(LDFLAGS) -o $@ $<
