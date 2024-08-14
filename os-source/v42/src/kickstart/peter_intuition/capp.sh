# $Id: capp.sh,v 38.0 91/06/12 14:14:01 peter Exp $
#   -- local BSR cappfile script  --
tee /tmp/capp$$a | grep "	XDEF	" | \
    sed -e "s/	XDEF	\([_A-Za-z0-9]*\).*/s\/JSR	\1$\/BSR	\1\//" \
    >/tmp/capp$$b
cat cappfile >> /tmp/capp$$b
sed -f /tmp/capp$$b /tmp/capp$$a
rm -f /tmp/capp$$a /tmp/capp$$b
