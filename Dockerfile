ARG BUILD_FROM
FROM $BUILD_FROM

ENV LANG C.UTF-8

ARG RPI_RGB_LED_MATRIX_VERSION
COPY postled /usr/src/postled
RUN apk add --no-cache mosquitto && \
    apk add --no-cache --virtual .build-dependencies build-base git
WORKDIR /usr/src/postled
RUN git clone https://github.com/hzeller/rpi-rgb-led-matrix.git  && \
    mv rpi-rgb-led-matrix matrix  && \
    git clone -b 6.18 https://github.com/cesanta/mongoose.git
RUN make 


# Copy data for add-on
COPY run.sh /
RUN chmod a+x /run.sh

CMD [ "/run.sh" ]

