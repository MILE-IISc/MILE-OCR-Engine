# X applications warn “Couldn't connect to accessibility bus:” on stderr
export NO_AT_BRIDGE=1

docker run -it --rm --name mile-ocr-engine-dev --net=host -v $PWD:/home/ocr mile-ocr-engine-dev bash
