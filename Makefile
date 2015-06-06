#Type 'nmake test' for testing
#or simply 'nmake' to build all targets

### Source dirs ###
ROOT=$(MAKEDIR)
SRC=$(ROOT)\src

###  3rd party libraries ###
HASHLIB=$(ROOT)\lib\hash-library
#header-only libraries:
TCLAP=$(ROOT)\lib\tclap-1.2.1\include
CATCH=$(ROOT)\lib\Catch\single_include
SPDLOG=$(ROOT)\lib\spdlog\include
CSV_PARSER=$(ROOT)\fast-cpp-csv-parser\include


### Object dependencies ###
SERVER_OBJS=server.obj server_loop.obj
CLIENT_OBJS=client.obj $(HASHLIB)\md5.obj
TESTS_OBJS=all_tests.obj $(HASHLIB)\md5.obj packets\common.obj \
		  packets\radius_packet.obj packets\eap_packet.obj \
		  radius_server.obj server_loop.obj


### Targets ###
SERVER=server.exe
CLIENT=client.exe
TESTS=all_tests.exe

### Include flags ###
COMMON_INC=/I$(HASHLIB) /I$(TCLAP) /I$(SRC) /I$(SPDLOG)

SERVER_INC=$(COMMON_INC) /I$(CSV_PARSER)
CLIENT_INC=$(COMMON_INC)
TESTS_INC=/I$(HASHLIB) /I$(CATCH) /I$(SRC) /I$(SPDLOG)

### Other flags ###
CFLAGS = /EHsc


all: $(SERVER) $(CLIENT) $(TESTS)

test: $(TESTS)
	$(TESTS)

$(SERVER): $(SERVER_OBJS)
	pushd $(SRC) & $(CC) $(CFLAGS) /Fe..\$@ $** & popd

$(CLIENT): $(CLIENT_OBJS)
	pushd $(SRC) & $(CC) $(CFLAGS) /Fe..\$@ $** & popd

$(TESTS): $(TESTS_OBJS)
	pushd $(SRC) & $(CC) $(CFLAGS) /Fe..\$@ $** & popd

server.obj: $(SRC)\server.cc
	pushd $(SRC) & $(CC) $(CFLAGS) $(SERVER_INC) -c $? \
		& popd

client.obj: $(SRC)\client.cc
	pushd $(SRC) & $(CC) $(CFLAGS) $(CLIENT_INC) -c $? \
		& popd

all_tests.obj: $(SRC)\all_tests.cc
	pushd $(SRC) & $(CC) $(CFLAGS) $(TESTS_INC) -c $? \
		& popd

clean:
	del *.obj *.exe $(SRC)\*.obj $(SRC)\packets\*.obj \
		$(HASHLIB)\*.obj



### Rules ### 
{$(HASHLIB)}.cpp{$(HASHLIB)}.obj::
	pushd $(HASHLIB) & $(CC) $(CFLAGS) /I$(HASHLIB) -c $< & popd

{$(SRC)}.cc{}.obj:
	pushd $(SRC) & $(CC) $(CFLAGS) $(COMMON_INC) -c $< & popd

{$(SRC)\packets}.cc{packets}.obj:
	pushd $(SRC)\packets & $(CC) $(CFLAGS) $(COMMON_INC) -c $< & popd

