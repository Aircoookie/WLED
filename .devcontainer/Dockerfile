#-------------------------------------------------------------------------------------------------------------
# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License. See https://go.microsoft.com/fwlink/?linkid=2090316 for license information.
#-------------------------------------------------------------------------------------------------------------

FROM mcr.microsoft.com/vscode/devcontainers/base:0-buster

# Configure apt and install packages
RUN apt-get update \
    && export DEBIAN_FRONTEND=noninteractive \
    #
    # Install C++ tools
    && apt-get -y install \
        # Common stuff
        apt-utils \
        build-essential \
        ca-certificates \
        curl \
        dialog \
        git \
        sudo \
        unzip \
        wget \
        # for platformio
        bc \
        build-essential \
        clang \
        curl \
        gcc \
        python3 \
        python3-dev \
        python3-distutils \
        python3-pip \
        srecord \
        xz-utils \
        build-essential cmake cppcheck valgrind python3-distutils nodejs npm \
    #
    # Clean up
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

RUN python3 -c "$(curl -fsSL https://raw.githubusercontent.com/platformio/platformio/develop/scripts/get-platformio.py)"