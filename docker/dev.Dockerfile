FROM ubuntu:22.04

RUN apt update --yes && apt install --yes ca-certificates curl gnupg

RUN echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-16 main" > /etc/apt/sources.list.d/llvm.list
RUN curl --silent --location https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -

RUN  mkdir -p /etc/apt/keyrings
RUN curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg

RUN echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_20.x nodistro main" | tee /etc/apt/sources.list.d/nodesource.list
RUN apt update --yes

RUN apt install --yes build-essential git cmake libssl-dev pip ninja-build gdb clangd-16 clang-format-16 nodejs zstd

RUN pip install conan==1.60.0
RUN conan profile new default --detect
RUN conan profile update settings.compiler.libcxx=libstdc++11 default

COPY conanfile.txt /conan/

RUN conan install /conan -pr=default -s build_type=Debug --build=missing -g txt -of /conan

RUN corepack enable
RUN corepack prepare yarn@stable --activate

EXPOSE 8080
