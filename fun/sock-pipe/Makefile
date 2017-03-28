WRITER_NAME=writer_ps
READER_NAME=reader_ps

2016:
	arm-marvell-linux-gnueabi-gcc -Wall $(WRITER_NAME).c -o $(WRITER_NAME) -lrt
	arm-marvell-linux-gnueabi-gcc -Wall $(READER_NAME).c -o $(READER_NAME) -lrt

mv:
	arm-mv5sft-linux-gnueabi-gcc -Wall $(WRITER_NAME).c -o $(WRITER_NAME) -lrt
	arm-mv5sft-linux-gnueabi-gcc -Wall $(READER_NAME).c -o $(READER_NAME) -lrt

host:
	gcc -Wall $(WRITER_NAME).c -o $(WRITER_NAME) -lrt
	gcc -Wall $(READER_NAME).c -o $(READER_NAME) -lrt

clean:
	rm -f $(WRITER_NAME) $(READER_NAME)X
