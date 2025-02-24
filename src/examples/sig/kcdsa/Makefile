ROOT_DIR=../../../..
BUILD_DIR=$(ROOT_DIR)/build
include $(ROOT_DIR)/common.mk

# Remove old includes with bad relative folders
CFLAGS := $(patsubst -I%,,$(CFLAGS))
# Add proper includes
CFLAGS += -I$(ROOT_DIR)/src/ -I$(ROOT_DIR)/src/external_deps

all:	kcdsa
ifeq ($(WITH_DYNAMIC_LIBS),1)
# If the user asked for dynamic libraries, compile versions of our binaries against them
all:	kcdsa_dyn
endif

libhash:
	# Compile the hashes
	cd ../../hash && EXTRA_CFLAGS="$(CFLAGS)" make libhash.a

kcdsa: libhash
	$(CC) $(BIN_CFLAGS) -DKCDSA kcdsa.c ../dsa/dsa.c $(ROOT_DIR)/src/external_deps/print.c $(ROOT_DIR)/src/external_deps/rand.c $(BIN_LDFLAGS) ../../hash/libhash.a $(LIBSIGN) -o kcdsa

# If the user asked for dynamic libraries, compile versions of our binaries against them
ifeq ($(WITH_DYNAMIC_LIBS),1)
kcdsa_dyn: libhash
	$(CC) $(BIN_CFLAGS) -DKCDSA kcdsa.c ../dsa/dsa.c $(ROOT_DIR)/src/external_deps/print.c $(ROOT_DIR)/src/external_deps/rand.c $(BIN_LDFLAGS) $(LIBEC) -L$(BUILD_DIR) ../../hash/libhash.a -lsign -o kcdsa_dyn
endif


clean:
	@rm -f kcdsa
	@rm -f kcdsa_dyn

.PHONY: all clean 16 32 64 debug debug16 debug32 debug64 force_arch32 force_arch64
