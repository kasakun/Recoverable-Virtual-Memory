CC = g++
CFLAGS = -g
LDFLAGS = -lrt -lm
LIBS = mylib/rvm.cpp
SRC = src/abort.cpp src/basic.cpp
OBJ = $(LIBS:.cpp=.o)

INCLUDE = -I mylib
OUT = bin/librvm.a

.c.o:
	@$(CC) $(CFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	@mkdir -p bin
	@ar rcs $(OUT) $(OBJ)

abort:
	@echo Building abort test...
	@mkdir -p bin/abort
	@$(CC) $(CFLAGS) $(INCLUDE) src/abort.cpp $(OUT) -o bin/abort/abort
	@echo Done!
	@echo Now run './bin/abort/abort'

abort_in_commit:
	@echo Building abort_in_commit test...
	@mkdir -p bin/abort_in_commit
	@$(CC) $(CFLAGS) $(INCLUDE) src/abort_in_commit.cpp $(OUT) -o bin/abort_in_commit/abort_in_commit
	@echo Done!
	@echo Now run './bin/abort_in_commit/abort_in_commit'

basic:
	@echo Building basic test...
	@mkdir -p bin/basic
	@$(CC) $(CFLAGS) $(INCLUDE) src/basic.cpp $(OUT) -o bin/basic/basic
	@echo Done!
	@echo Now run './bin/basic/basic'

multi:
	@echo Building multi test...
	@mkdir -p bin/multi
	@$(CC) $(CFLAGS) $(INCLUDE) src/multi.cpp $(OUT) -o bin/multi/multi
	@echo Done!
	@echo Now run './bin/multi/multi'

multi-abort:
	@echo Building multi-abort test...
	@mkdir -p bin/multi-abort
	@$(CC) $(CFLAGS) $(INCLUDE) src/multi-abort.cpp $(OUT) -o bin/multi-abort/multi-abort
	@echo Done!
	@echo Now run './bin/multi-abort/multi-abort'

multi_trans_basic:
	@echo Building multi_trans_basic test...
	@mkdir -p bin/multi_trans_basic
	@$(CC) $(CFLAGS) $(INCLUDE) src/multi_trans_basic.cpp $(OUT) -o bin/multi_trans_basic/multi_trans_basic
	@echo Done!
	@echo Now run './bin/multi_trans_basic/multi_trans_basic'

multi_trans_crash:
	@echo Building multi_trans_crash_basic test...
	@mkdir -p bin/multi_trans_crash_basic
	@$(CC) $(CFLAGS) $(INCLUDE) src/multi_trans_crash_basic.cpp $(OUT) -o bin/multi_trans_crash_basic/multi_trans_crash_basic
	@echo Done!
	@echo Now run './bin/multi_trans_crash/multi_trans_crash'

truncate:
	@echo Building truncate test...
	@mkdir -p bin/truncate
	@$(CC) $(CFLAGS) $(INCLUDE) src/truncate.cpp $(OUT) -o bin/truncate/truncate
	@echo Done!
	@echo Now run './bin/truncate/truncate'

all:
	@echo Building All...
	@mkdir -p bin/abort
	@$(CC) $(CFLAGS) $(INCLUDE) src/abort.cpp $(OUT) -o bin/abort/abort
	@mkdir -p bin/abort_in_commit
	@$(CC) $(CFLAGS) $(INCLUDE) src/abort_in_commit.cpp $(OUT) -o bin/abort_in_commit/abort_in_commit
	@mkdir -p bin/basic
	@$(CC) $(CFLAGS) $(INCLUDE) src/basic.cpp $(OUT) -o bin/basic/basic
	@mkdir -p bin/multi
	@$(CC) $(CFLAGS) $(INCLUDE) src/multi.cpp $(OUT) -o bin/multi/multi
	@mkdir -p bin/multi-abort
	@$(CC) $(CFLAGS) $(INCLUDE) src/multi-abort.cpp $(OUT) -o bin/multi-abort/multi-abort
	@mkdir -p bin/multi_trans_basic
	@$(CC) $(CFLAGS) $(INCLUDE) src/multi_trans_basic.cpp $(OUT) -o bin/multi_trans_basic/multi_trans_basic
	@mkdir -p bin/multi_trans_crash_basic
	@$(CC) $(CFLAGS) $(INCLUDE) src/multi_trans_crash_basic.cpp $(OUT) -o bin/multi_trans_crash_basic/multi_trans_crash_basic
	@mkdir -p bin/truncate
	@$(CC) $(CFLAGS) $(INCLUDE) src/truncate.cpp $(OUT) -o bin/truncate/truncate
	@echo Done!

clean :
	@rm mylib/*.o bin/*.a
	@rm -r bin
	@echo Cleaned!
