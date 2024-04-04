# Base Stage: Prepare all dependencies and tools
FROM ubuntu:22.04 as base

RUN apt-get update --yes && apt-get install --yes ca-certificates curl gnupg \
    && echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-17 main" > /etc/apt/sources.list.d/llvm.list \
    && curl --silent --location https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - \
    && mkdir -p /etc/apt/keyrings \
    && curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg \
    && echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_20.x nodistro main" | tee /etc/apt/sources.list.d/nodesource.list \
    && apt-get update --yes \
    && apt-get install --yes build-essential git cmake libssl-dev pip ninja-build gdb clangd-17 clang-format-17 clang-17 nodejs zstd \
    && rm -rf /var/lib/apt/lists/*

RUN pip install conan==1.63.0 \
    && conan profile new default --detect \
    && conan profile update settings.compiler.libcxx=libstdc++11 default

RUN corepack enable \
    && corepack prepare yarn@stable --activate

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
    && apt-get install --yes ca-certificates curl gnupg zstd \
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
