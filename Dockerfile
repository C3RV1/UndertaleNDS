FROM archlinux:latest
WORKDIR /tmp

# Install dependencies

ENV DEVKITPRO=/opt/devkitpro
ENV DEVKITARM=/opt/devkitpro/devkitARM

RUN pacman-key --init
RUN pacman -Syyuu --noconfirm \
    curl \
    make 
RUN pacman-key --recv BC26F752D25B92CE272E0F44F7FD5492264BB9D0 --keyserver keyserver.ubuntu.com
RUN pacman-key --recv 032f4c9d0b8ff3bc84019b7862c7609ada219c60 --keyserver keyserver.ubuntu.com
RUN pacman-key --lsign BC26F752D25B92CE272E0F44F7FD5492264BB9D0
RUN pacman-key --lsign 032f4c9d0b8ff3bc84019b7862c7609ada219c60
RUN curl -O https://pkg.devkitpro.org/devkitpro-keyring.pkg.tar.xz
RUN printf '\n[dkp-libs]\nServer = https://pkg.devkitpro.org/packages\n' >> /etc/pacman.conf
RUN printf '\n[dkp-linux]\nServer = https://pkg.devkitpro.org/packages/linux/$arch/\n' >> /etc/pacman.conf
RUN pacman -Sy
RUN pacman -Syyuu --noconfirm \
    nds-dev

# Build commands

WORKDIR /opt/workdir
