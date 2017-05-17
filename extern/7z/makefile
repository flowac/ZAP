PROG = 7z.so
CXX = g++ -O2 -fpic
# -Wall -Werror -Wno-delete-non-virtual-dtor
CXX_C = gcc -O2 -Wall -Werror -fpic

ifdef SystemDrive
IS_MINGW = 1
endif

ifdef IS_MINGW

RM = del
CFLAGS = -c
LIB2 = -loleaut32 -luuid
LDFLAGS = -s

FILE_IO =FileIO

MT_FILES = \
  LzFindMt.o \
  Threads.o \

else

RM = rm -f
CFLAGS = -c -D_7ZIP_ST

FILE_IO =C_FileIO

endif


OBJS = \
  $(MT_FILES) \
  $(FILE_IO).o \
  LzmaAlone.o \
  Bench.o \
  BenchCon.o \
  ConsoleClose.o \
  LzmaDecoder.o \
  LzmaEncoder.o \
  LzmaRegister.o \
  CreateCoder.o \
  CWrappers.o \
  FileStreams.o \
  FilterCoder.o \
  MethodProps.o \
  StreamUtils.o \
  CommandLineParser.o \
  CRC.o \
  CrcReg.o \
  IntToString.o \
  MyString.o \
  MyVector.o \
  MyWindows.o \
  StringConvert.o \
  StringToInt.o \
  PropVariant.o \
  System.o \
  7zCrc.o \
  7zCrcOpt.o \
  Alloc.o \
  Bra86.o \
  CpuArch.o \
  LzFind.o \
  LzmaDec.o \
  LzmaEnc.o \
  Lzma86Dec.o \
  Lzma86Enc.o \


all: $(PROG)

$(PROG): $(OBJS)
	$(CXX) -shared -o ../../lib/$(PROG) $(LDFLAGS) $(OBJS) $(LIB2) 

LzmaAlone.o: LzmaAlone.cpp
	$(CXX) $(CFLAGS) LzmaAlone.cpp

Bench.o: Bench.cpp
	$(CXX) $(CFLAGS) Bench.cpp

BenchCon.o: BenchCon.cpp
	$(CXX) $(CFLAGS) BenchCon.cpp

ConsoleClose.o: ConsoleClose.cpp
	$(CXX) $(CFLAGS) ConsoleClose.cpp

LzmaDecoder.o: LzmaDecoder.cpp
	$(CXX) $(CFLAGS) LzmaDecoder.cpp

LzmaEncoder.o: LzmaEncoder.cpp
	$(CXX) $(CFLAGS) LzmaEncoder.cpp

LzmaRegister.o: LzmaRegister.cpp
	$(CXX) $(CFLAGS) LzmaRegister.cpp

CreateCoder.o: CreateCoder.cpp
	$(CXX) $(CFLAGS) CreateCoder.cpp

CWrappers.o: CWrappers.cpp
	$(CXX) $(CFLAGS) CWrappers.cpp

FileStreams.o: FileStreams.cpp
	$(CXX) $(CFLAGS) FileStreams.cpp

FilterCoder.o: FilterCoder.cpp
	$(CXX) $(CFLAGS) FilterCoder.cpp

MethodProps.o: MethodProps.cpp
	$(CXX) $(CFLAGS) MethodProps.cpp

StreamUtils.o: StreamUtils.cpp
	$(CXX) $(CFLAGS) StreamUtils.cpp

$(FILE_IO).o: $(FILE_IO).cpp
	$(CXX) $(CFLAGS) $(FILE_IO).cpp


CommandLineParser.o: CommandLineParser.cpp
	$(CXX) $(CFLAGS) CommandLineParser.cpp

CRC.o: CRC.cpp
	$(CXX) $(CFLAGS) CRC.cpp

CrcReg.o: CrcReg.cpp
	$(CXX) $(CFLAGS) CrcReg.cpp

IntToString.o: IntToString.cpp
	$(CXX) $(CFLAGS) IntToString.cpp

MyString.o: MyString.cpp
	$(CXX) $(CFLAGS) MyString.cpp

MyVector.o: MyVector.cpp
	$(CXX) $(CFLAGS) MyVector.cpp

MyWindows.o: MyWindows.cpp
	$(CXX) $(CFLAGS) MyWindows.cpp

StringConvert.o: StringConvert.cpp
	$(CXX) $(CFLAGS) StringConvert.cpp

StringToInt.o: StringToInt.cpp
	$(CXX) $(CFLAGS) StringToInt.cpp

PropVariant.o: PropVariant.cpp
	$(CXX) $(CFLAGS) PropVariant.cpp

System.o: System.cpp
	$(CXX) $(CFLAGS) System.cpp

7zCrc.o: C/7zCrc.c
	$(CXX_C) $(CFLAGS) C/7zCrc.c

7zCrcOpt.o: C/7zCrcOpt.c
	$(CXX_C) $(CFLAGS) C/7zCrcOpt.c

Alloc.o: C/Alloc.c
	$(CXX_C) $(CFLAGS) C/Alloc.c

Bra86.o: C/Bra86.c
	$(CXX_C) $(CFLAGS) C/Bra86.c

CpuArch.o: C/CpuArch.c
	$(CXX_C) $(CFLAGS) C/CpuArch.c

LzFind.o: C/LzFind.c
	$(CXX_C) $(CFLAGS) C/LzFind.c

ifdef MT_FILES
LzFindMt.o: C/LzFindMt.c
	$(CXX_C) $(CFLAGS) C/LzFindMt.c

Threads.o: C/Threads.c
	$(CXX_C) $(CFLAGS) C/Threads.c
endif

LzmaDec.o: C/LzmaDec.c
	$(CXX_C) $(CFLAGS) C/LzmaDec.c

LzmaEnc.o: C/LzmaEnc.c
	$(CXX_C) $(CFLAGS) C/LzmaEnc.c

Lzma86Dec.o: C/Lzma86Dec.c
	$(CXX_C) $(CFLAGS) C/Lzma86Dec.c

Lzma86Enc.o: C/Lzma86Enc.c
	$(CXX_C) $(CFLAGS) C/Lzma86Enc.c

clean:
	-$(RM) $(PROG) $(OBJS)
