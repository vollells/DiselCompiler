FROM ubuntu:bionic
# FROM sjoelund/bionic-sphinx:latest
MAINTAINER Martin Sjölund <martin.sjolund@liu.se>
ENV DEBIAN_FRONTEND noninteractive

# The ubuntu:bionic image ignores documentation, so extract the documentation manually
RUN apt-get update -q && apt-get upgrade -qy && apt-get install -qy \
    texlive-base texlive-latex-extra latexmk \
    python3 python3-pip gnuplot doxygen \
    flex bison graphviz \
    make git \
    && apt-get download flex-doc bison-doc gcc-doc gcc-7-doc \
    && ar x flex-doc*.deb data.tar.xz && tar xJf data.tar.xz -C / \
    && ar x bison-doc*.deb data.tar.xz && tar xJf data.tar.xz -C / \
    && ar x gcc-doc*.deb data.tar.xz && tar xJf data.tar.xz -C / \
    && ar x gcc-7-doc*.deb data.tar.xz && tar xJf data.tar.xz -C / \
    && rm -rf /var/lib/apt/lists/* *.deb *.tar.xz \
    && apt-get clean \
    && pip3 install --upgrade breathe "sphinx>=1.6.5,<1.7.0" sphinxcontrib-bibtex junit2html junit-xml sphinx-git sphinx_rtd_theme sphinxcontrib-bibtex

WORKDIR /data
VOLUME ["/data"]
