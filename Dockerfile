#syntax=docker/dockerfile:1.9
ARG LLVM_VERSION=20
ARG USERNAME=developer
ARG USER_UID=1000
ARG USER_GID=1000

# ╔══════════════════════════════════════════════════════════════════════════════╗
# ║                                  BASE STAGE                                  ║
# ║                    Prepare all dependencies and tools                        ║
# ╚══════════════════════════════════════════════════════════════════════════════╝
FROM ubuntu:24.04 AS base
ARG LLVM_VERSION
ARG USERNAME
ARG USER_UID
ARG USER_GID

ENV DEBIAN_FRONTEND=noninteractive

# ──────────────────────────────────────────────────────────────────────────────
#  Remove default ubuntu user to avoid UID/GID conflicts
# ──────────────────────────────────────────────────────────────────────────────
RUN <<EOF
# Remove default ubuntu user and group if they exist
if id -u ubuntu >/dev/null 2>&1; then
    userdel -r ubuntu 2>/dev/null || true
fi
if getent group ubuntu >/dev/null 2>&1; then
    groupdel ubuntu 2>/dev/null || true
fi
# Remove any existing user/group with target UID/GID
if id -u ${USER_UID} >/dev/null 2>&1; then
    existing_user=$(id -nu ${USER_UID})
    userdel -r ${existing_user} 2>/dev/null || true
fi
if getent group ${USER_GID} >/dev/null 2>&1; then
    existing_group=$(getent group ${USER_GID} | cut -d: -f1)
    groupdel ${existing_group} 2>/dev/null || true
fi
EOF

# ──────────────────────────────────────────────────────────────────────────────
#  Install essential packages and repositories
# ──────────────────────────────────────────────────────────────────────────────
RUN <<EOF
apt-get update --yes
apt-get install --yes \
    ca-certificates \
    curl \
    gnupg \
    software-properties-common
EOF

# ──────────────────────────────────────────────────────────────────────────────
# Setup package repositories (LLVM, CMake & XMake)
# ──────────────────────────────────────────────────────────────────────────────
RUN <<EOF
mkdir -p /etc/apt/keyrings
curl --silent --location https://apt.llvm.org/llvm-snapshot.gpg.key | gpg --dearmor -o /etc/apt/keyrings/llvm-archive-keyring.gpg
echo "deb [signed-by=/etc/apt/keyrings/llvm-archive-keyring.gpg] http://apt.llvm.org/noble/ llvm-toolchain-noble-${LLVM_VERSION} main" > /etc/apt/sources.list.d/llvm.list
curl --silent --location https://apt.kitware.com/keys/kitware-archive-latest.asc | gpg --dearmor -o /etc/apt/keyrings/kitware-archive-keyring.gpg
echo "deb [signed-by=/etc/apt/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ noble main" > /etc/apt/sources.list.d/kitware.list
add-apt-repository ppa:xmake-io/xmake --yes
apt-get update --yes
EOF

# ──────────────────────────────────────────────────────────────────────────────
# Install development tools and LLVM toolchain
# ──────────────────────────────────────────────────────────────────────────────
RUN <<EOF
apt-get install --yes \
    build-essential \
    clang-${LLVM_VERSION} \
    clang-format-${LLVM_VERSION} \
    clang-tidy-${LLVM_VERSION} \
    clangd-${LLVM_VERSION} \
    cmake \
    curl \
    git \
    gnupg \
    libc++-${LLVM_VERSION}-dev \
    libc++abi-${LLVM_VERSION}-dev \
    libssl-dev \
    lldb-${LLVM_VERSION} \
    ninja-build \
    nodejs \
    p7zip-full \
    pkg-config \
    sudo \
    unzip \
    xmake \
    zip \
    zstd \
    
rm -rf /var/lib/apt/lists/*
EOF

# ──────────────────────────────────────────────────────────────────────────────
#  Install Bun runtime globally
# ──────────────────────────────────────────────────────────────────────────────
RUN curl -fsSL https://bun.sh/install | bash && \
    cp /root/.bun/bin/bun /usr/local/bin/ && \
    cp /root/.bun/bin/bunx /usr/local/bin/

# ──────────────────────────────────────────────────────────────────────────────
#  Setup compiler alternatives
# ──────────────────────────────────────────────────────────────────────────────
RUN <<EOF
update-alternatives --install /usr/bin/clang clang /usr/bin/clang-${LLVM_VERSION} 100
update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-${LLVM_VERSION} 100
update-alternatives --install /usr/bin/cc cc /usr/bin/clang-${LLVM_VERSION} 100
update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-${LLVM_VERSION} 100
update-alternatives --set clang /usr/bin/clang-${LLVM_VERSION}
update-alternatives --set clang++ /usr/bin/clang++-${LLVM_VERSION}
update-alternatives --set cc /usr/bin/clang-${LLVM_VERSION}
update-alternatives --set c++ /usr/bin/clang++-${LLVM_VERSION}
EOF

ENV PATH="/usr/local/bin:/root/.local/bin:${PATH}"

# ──────────────────────────────────────────────────────────────────────────────
#  Create non-root user for development
# ──────────────────────────────────────────────────────────────────────────────
RUN <<EOF
# Create user with fallback strategy
if id -u ${USERNAME} >/dev/null 2>&1; then
    echo "User ${USERNAME} already exists"
else
    # Try with specified UID/GID first, then fallback to auto-assigned
    (groupadd --gid ${USER_GID} ${USERNAME} 2>/dev/null || groupadd ${USERNAME}) && \
    (useradd --uid ${USER_UID} --gid ${USERNAME} --shell /bin/bash --create-home ${USERNAME} 2>/dev/null || \
     useradd --gid ${USERNAME} --shell /bin/bash --create-home ${USERNAME})
fi

# Ensure user is in sudo group
usermod -aG sudo ${USERNAME}
echo "${USERNAME} ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
EOF


# ╔══════════════════════════════════════════════════════════════════════════════╗
# ║                              DEVELOPMENT STAGE                               ║
# ║                    Setup rootless development environment                    ║
# ╚══════════════════════════════════════════════════════════════════════════════╝

FROM base AS development
ARG USERNAME
ARG USER_UID
ARG USER_GID

# ──────────────────────────────────────────────────────────────────────────────
#  Setup user environment
# ──────────────────────────────────────────────────────────────────────────────
USER ${USERNAME}
WORKDIR /home/${USERNAME}

ENV PATH="/usr/local/bin:/home/${USERNAME}/.local/bin:${PATH}"

# ──────────────────────────────────────────────────────────────────────────────
#  Network ports
# ──────────────────────────────────────────────────────────────────────────────
# EXPOSE 8080
# EXPOSE 3030

# ──────────────────────────────────────────────────────────────────────────────
# 📂 Setup working directory with proper permissions
# ──────────────────────────────────────────────────────────────────────────────
RUN mkdir -p /home/${USERNAME}/workspace
WORKDIR /home/${USERNAME}/workspace

# ╔══════════════════════════════════════════════════════════════════════════════╗
# ║                           PRODUCTION BUILD STAGE                             ║
# ║                     Build the application for production                     ║
# ╚══════════════════════════════════════════════════════════════════════════════╝

FROM base AS production-build
ARG USERNAME

# ──────────────────────────────────────────────────────────────────────────────
#  Copy source code and set proper permissions
# ──────────────────────────────────────────────────────────────────────────────
COPY --chown=${USERNAME}:${USERNAME} ./ /src
WORKDIR /src

# ──────────────────────────────────────────────────────────────────────────────
#  Build with XMake (as non-root user)
# ──────────────────────────────────────────────────────────────────────────────
USER ${USERNAME}
RUN <<EOF
# Configure XMake for release build
xmake config --mode=release --toolchain=clang
# Build the project
xmake build
# Create build output directory with proper permissions
sudo mkdir -p /build
sudo chown ${USERNAME}:${USERNAME} /build
# Install to /build directory
xmake install --installdir=/build
EOF

# ╔══════════════════════════════════════════════════════════════════════════════╗
# ║                             PRODUCTION STAGE                                 ║
# ║                    Minimal runtime environment                               ║
# ╚══════════════════════════════════════════════════════════════════════════════╝

FROM ubuntu:24.04 AS production
ARG LLVM_VERSION=18

# ──────────────────────────────────────────────────────────────────────────────
#  Install minimal runtime dependencies
# ──────────────────────────────────────────────────────────────────────────────
RUN <<EOF
apt-get update --yes
apt-get install --yes \
    ca-certificates \
    curl \
    gnupg
mkdir -p /etc/apt/keyrings
curl --silent --location https://apt.llvm.org/llvm-snapshot.gpg.key | gpg --dearmor -o /etc/apt/keyrings/llvm-archive-keyring.gpg
echo "deb [signed-by=/etc/apt/keyrings/llvm-archive-keyring.gpg] http://apt.llvm.org/noble/ llvm-toolchain-noble-${LLVM_VERSION} main" > /etc/apt/sources.list.d/llvm.list
apt-get update --yes
apt-get install --yes \
    bzip2 \
    libc++1-${LLVM_VERSION} \
    libc++abi1-${LLVM_VERSION} \
    liblz4-1 \
    xz-utils \
    zlib1g \
    zstd
rm -rf /var/lib/apt/lists/*
EOF

# ──────────────────────────────────────────────────────────────────────────────
#  Copy application and setup runtime user
# ──────────────────────────────────────────────────────────────────────────────
COPY --from=production-build /build /app

RUN <<EOF
mkdir -p /app/persistence/
adduser --disabled-password --gecos '' bxt
chown -R bxt:bxt /app
chmod 755 /app
EOF

# ──────────────────────────────────────────────────────────────────────────────
#  Runtime configuration
# ──────────────────────────────────────────────────────────────────────────────
USER bxt
ENV LD_LIBRARY_PATH=/app/lib/
WORKDIR /app
EXPOSE 8080

CMD ["/app/bxtd"]
