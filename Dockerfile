FROM mile-ocr-engine-base

WORKDIR /home/ocr
COPY Makefile /home/ocr/
COPY src /home/ocr/src/
COPY etc /home/ocr/etc/

# Build project
RUN make

CMD ["/home/ocr/KannadaClassifier.exe", "-server"]

