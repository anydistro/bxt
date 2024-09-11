#syntax=docker/dockerfile:1.9
################################################################################
# Base Stage: Prepare all dependencies and tools
################################################################################
FROM ubuntu:24.04 as base

ENV DEBIAN_FRONTEND=noninteractive
RUN <<EOF
apt-get update --yes
apt-get install --yes \
    ca-certificates \
    curl \
    gnupg
mkdir -p /etc/apt/keyrings
curl --silent --location https://apt.llvm.org/llvm-snapshot.gpg.key | gpg --dearmor -o /etc/apt/keyrings/llvm-archive-keyring.gpg
curl --silent --location https://apt.kitware.com/keys/kitware-archive-latest.asc | gpg --dearmor -o /etc/apt/keyrings/kitware-archive-keyring.gpg
echo "deb [signed-by=/etc/apt/keyrings/llvm-archive-keyring.gpg] http://apt.llvm.org/noble/ llvm-toolchain-noble-18 main" > /etc/apt/sources.list.d/llvm.list
echo "deb [signed-by=/etc/apt/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ noble main" > /etc/apt/sources.list.d/kitware.list
apt-get update --yes
apt-get install --yes \
    build-essential \
    clang-18 \
    clang-format-18 \
    clangd-18 \
    cmake \
    curl \
    git \
    gnupg \
    libc++-18-dev \
    libc++abi-18-dev \
    libssl-dev \
    lldb-18 \
    ninja-build \
    nodejs \
    pipx \
    unzip \
    zstd
rm -rf /var/lib/apt/lists/*
EOF

RUN <<EOF
update-alternatives --install /usr/bin/clang clang /usr/bin/clang-18 100
update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-18 100
update-alternatives --install /usr/bin/cc cc /usr/bin/clang-18 100
update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-18 100
update-alternatives --set clang /usr/bin/clang-18
update-alternatives --set clang++ /usr/bin/clang++-18
update-alternatives --set cc /usr/bin/clang-18
update-alternatives --set c++ /usr/bin/clang++-18
EOF

ENV PATH="/root/.bun/bin:/root/.local/bin:${PATH}"

RUN pipx install conan==2.7.0

RUN conan profile detect --force --name default && \
    cat <<EOF >> /root/.conan2/profiles/default 
compiler=clang 
compiler.version=18 
compiler.libcxx=libc++ 
compiler.cppstd=23
EOF

RUN git clone https://github.com/conan-io/cmake-conan.git -b develop2 /cmake-conan

RUN curl -fsSL https://bun.sh/install | bash


COPY conanfile.py /conan/

################################################################################
# Development Stage: Set up the development environment
################################################################################
FROM base as development

RUN conan install /conan -pr=/root/.conan2/profiles/default  -s build_type=Debug --build=missing -of /conan
# for main application
EXPOSE 8080 
# for web server
EXPOSE 3000

################################################################################
# Production Build Stage: Build the application for production
################################################################################
FROM base as production-build

RUN pipx ensurepath && conan install /conan -pr=default -s build_type=Release --build=missing -of /conan
COPY ./ /src

RUN <<EOF
cmake -B/build -S/src -DCMAKE_BUILD_TYPE=Release --preset release
cmake --build /build
EOF

################################################################################
# Production Stage: Set up the environment for running the application
################################################################################
FROM ubuntu:24.04 as production

RUN <<EOF
apt-get update --yes
apt-get install --yes \
    ca-certificates \
    curl \
    gnupg
mkdir -p /etc/apt/keyrings
curl --silent --location https://apt.llvm.org/llvm-snapshot.gpg.key | gpg --dearmor -o /etc/apt/keyrings/llvm-archive-keyring.gpg
echo "deb [signed-by=/etc/apt/keyrings/llvm-archive-keyring.gpg] http://apt.llvm.org/jammy/ llvm-toolchain-jammy-18 main" > /etc/apt/sources.list.d/llvm.list
apt-get update --yes
apt-get install --yes \
    bzip2 \
    libc++1-18 \
    libc++abi1-18 \
    liblz4-1 \
    xz-utils \
    zlib1g \
    zstd
rm -rf /var/lib/apt/lists/*
EOF

COPY --from=production-build /build/bin /app

RUN <<EOF
mkdir -p /app/persistence/
adduser --disabled-password --gecos '' bxt
chown -R bxt:bxt /app
chmod 755 /app
EOF

USER bxt

ENV LD_LIBRARY_PATH=/app/lib/
WORKDIR /app
EXPOSE 8080
CMD ["/app/bxtd"]
