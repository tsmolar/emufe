rm -f *~

objlist=""
proglist="button window widget font"

for prog in $proglist
do
  echo "Building $prog.c"
  gcc -c -DALLEGRO $prog.c
  objlist="$objlist $prog.o"
done

echo linking $objlist
ar rc libwidget.a $objlist
ranlib libwidget.a
ar rc libfont.a font.o
ranlib libfont.a

#gcc -o tileclip `allegro-config --libs` tileclip.o font.o widget.o button.o window.o
#gcc -o balldemo `allegro-config --libs` isoeng.o legacyio.o balldemo.o
#gcc -o mapedit `allegro-config --libs` isoeng.o legacyio.o mapedit.o
