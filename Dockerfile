FROM mileiisc/tensorflow-with-opencv

WORKDIR /home/ocr
COPY Makefile /home/ocr/
COPY src /home/ocr/src/

COPY etc /home/ocr/etc/
COPY kn_model /home/ocr/kn_model

# Build project
RUN make

CMD ["/home/ocr/KannadaClassifier.exe", "-server"]

