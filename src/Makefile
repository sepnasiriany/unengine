HEADERS := $(wildcard *.h)
SOURCES := $(wildcard *.cpp)

format: $(HEADERS) $(SOURCES)
	clang-format $(HEADERS) $(SOURCES) -i --style=Google

verify-format: 
	clang-format $(HEADERS) $(SOURCES)  --dry-run --Werror --style=Google
