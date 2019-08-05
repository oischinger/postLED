ARG BUILD_FROM
FROM $BUILD_FROM

ENV LANG C.UTF-8

ARG RPI_RGB_LED_MATRIX_VERSION
WORKDIR /usr/src
COPY postled /usr/src/postled        
RUN apk add --no-cache \
        mosquitto \
    && apk add --no-cache --virtual .build-dependencies \
        build-base git \
    && cd postled \
    && git clone https://github.com/hzeller/rpi-rgb-led-matrix.git \
    && mv rpi-rgb-led-matrix matrix \
    && git clone https://github.com/cesanta/mongoose.git \
    && make 


# Copy data for add-on
COPY run.sh /
RUN chmod a+x /run.sh

CMD [ "/run.sh" ]

