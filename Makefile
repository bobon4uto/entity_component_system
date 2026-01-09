TARGET = build/main


all: build run

build:
	echo build

run:
	./nob

clean:
	rm -f $(TARGET)
