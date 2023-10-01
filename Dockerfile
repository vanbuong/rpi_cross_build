FROM sdthirlwall/raspberry-pi-cross-compiler

WORKDIR /build

RUN echo "while true; do sleep 1; done" > sleep.sh
RUN chmod +x /build/sleep.sh

CMD ["/bin/bash", "/build/sleep.sh"]