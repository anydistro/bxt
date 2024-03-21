FROM bxt:dev

COPY ./ /src

RUN cmake -B/build -S/src -DCMAKE_BUILD_TYPE=Release -G Ninja
RUN cmake --build /build
RUN cp /src/daemon/box.yml /build/bin/
ENV LD_LIBRARY_PATH=./libs/

WORKDIR /build/bin/

CMD ["./bxtd"]
