#!/bin/sh -ex

(cd instructions && (test -d x86doc || git clone https://github.com/HJLebbink/x86doc.git) && cd x86doc && git checkout 0d6973502e2c60a6752046ecf8a7d7b58c2bf08b)
test -f instructions/source/_static/x86doc/ADD.html
(cd instructions && doxygen)
make -C instructions html
touch instructions/source/*.rst
make -C instructions html # Re-build to find all doxygen links
test -f instructions/build/html/_static/x86doc/ADD.html
(
cd instructions/build/html/_static/x86doc
(
echo '<html><body>'
ls | sed 's/^.*/<a href="&">&<\/a><br\/>/'
echo '</body></html>'
) > index.html
)
