# Base Stage: Prepare all dependencies and tools
FROM ubuntu:22.04 as base

RUN apt-get update --yes && apt-get install --yes ca-certificates curl gnupg \
    && echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-17 main" > /etc/apt/sources.list.d/llvm.list \
    && curl --silent --location https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - \
    && mkdir -p /etc/apt/keyrings \
    && curl --silent --location https://apt.kitware.com/keys/kitware-archive-latest.asc | gpg --dearmor -o /usr/share/keyrings/kitware-archive-keyring.gpg \
    &&  echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null \
    && apt-get update --yes \
    && apt-get install --yes build-essential git cmake libssl-dev pip ninja-build lldb-17 clangd-17 clang-format-17 clang-17 libc++-17-dev libc++abi-17-dev nodejs zstd unzip \
    && rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-17 100 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-17 100 && \
    update-alternatives --install /usr/bin/cc cc /usr/bin/clang-17 100 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-17 100 && \
    update-alternatives --set clang /usr/bin/clang-17 && \
    update-alternatives --set clang++ /usr/bin/clang++-17 && \
    update-alternatives --set cc /usr/bin/clang-17 && \
    update-alternatives --set c++ /usr/bin/clang++-17

RUN pip install conan==1.63.0 \
    && conan profile new default --detect \
    && conan profile update settings.compiler=clang default \
    && conan profile update settings.compiler.version=17 default \
    && conan profile update settings.compiler.libcxx=libc++ default

RUN curl -fsSL https://bun.sh/install | bash

ENV PATH="/root/.bun/bin:${PATH}"

COPY conanfile.txt /conan/

# Development Stage: Set up the development environment
FROM base as development

RUN conan install /conan -pr=default -s build_type=Debug --build=missing -g txt -of /conan
EXPOSE 8080

# Production Build Stage: Build the application for production
FROM base as production-build

RUN conan install /conan -pr=default -s build_type=Release --build=missing -g txt -of /conan
COPY ./ /src

RUN cmake -B/build -S/src -DCMAKE_BUILD_TYPE=Release -G Ninja \
    && cmake --build /build \
    && cp /src/configs/box.yml /build/bin/ \
    && cp /src/configs/config.toml /build/bin/ 

# Production Stage: Set up the environment for running the application
FROM ubuntu:22.04 as production

RUN apt-get update --yes \
    && apt-get install --yes ca-certificates curl gnupg \
   	&& echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-17 main" > /etc/apt/sources.list.d/llvm.list \
    && curl --silent --location https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - \
    && apt-get update && apt-get install --yes bzip2 xz-utils zlib1g liblz4-1 zstd libc++1-17 libc++abi1-17 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=production-build /build/bin /app

RUN mkdir -p /app/persistence/ && \
    adduser --disabled-password --gecos '' bxt \
    && chown -R bxt:bxt /app \
    && chmod 755 /app 
    

USER bxt

ENV LD_LIBRARY_PATH=/app/libs/
WORKDIR /app
EXPOSE 8080
CMD ["/app/bxtd"]
