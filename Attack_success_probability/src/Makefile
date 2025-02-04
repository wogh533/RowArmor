CXX = g++
CXXFLAGS = -O3
BINDIR = ../bin
OBJDIR = ../obj
TARGET = $(BINDIR)/main
SRCS = main.cc BER_data.cc

OBJS = $(patsubst %.cc, $(OBJDIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)


$(OBJDIR)/%.o: %.cc
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean