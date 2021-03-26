# MILE-OCR-Engine

## Build docker image
docker build -t mile-ocr-engine

## Run container
$ docker run -d --name ocr-api -p 9080:9080 -p 9443:9443 mile-ocr-api
$ docker ps
