# figure out emuhome and stuff


CDR=$(dirname $0)
cd $CDR;emuhome=$PWD;cd -
#echo $emuhome
#exit

widesceen=0
type=3

if [ "$(uname -s)" = "CYGWIN_NT-5.1" ]; then
  EMUBOOKMARK=C:\\cygwin\\tmp\\mybm
else
  EMUBOOKMARK=$(mktemp)
fi
export EMUBOOKMARK
WS=""
if [ "$widescreen" = "1" ]; then WS="-ws -res 840x525"; fi


if [ "$type" = "4" ]; then
  $emuhome/emufe -n -i -ac $WS
fi

#cc=1
if [ "$type" = "3" ]; then
  exe="$emuhome/emufe -n -i $WS -ac -nx "
#  echo "$exe "
  $exe
#  cp stdout.txt output${cc}.txt
#  ((cc++))
  while [ 1 ]; do
    cmd=$(grep ^exec $EMUBOOKMARK | cut -d\| -f2)
    echo $EMUBOOKMARK
    cat $EMUBOOKMARK
    if [ "$cmd" = "QUIT" ]; then break; fi
    echo "exe $cmd"
    $cmd
    $exe -b $EMUBOOKMARK
#    $emuhome/emufe -n -i $WS -ac -nx -b $EMUBOOKMARK
#    cp stdout.txt output${cc}.txt
#    ((cc++))
  done
fi
