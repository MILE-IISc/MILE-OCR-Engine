FROM ubuntu:16.04

# install dependencies
RUN apt-get update
ENV DEBIAN_FRONTEND="noninteractive" TZ="Asia/Kolkata"
RUN apt-get install -y libopencv-dev liblinear-dev libsvm-dev libtiff5-dev

RUN apt-get install -y build-essential checkinstall cmake

WORKDIR /home/ocr
COPY Makefile /home/ocr/
COPY src /home/ocr/src/
COPY etc /home/ocr/etc/

# Build project
RUN make

CMD ["/home/ocr/KannadaClassifier.exe", "-server"]

