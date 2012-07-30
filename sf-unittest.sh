#/bin/sh

if [ $# -ne 1 ] ; then
	echo "Usage: $0 <sf.name>";
	exit;
fi

sed -n '/\/* Output/,\/*\// { /\* Output/d ; /*\//d ; p } ' $1 > $1.~ut
./src/saffire $1 | php ./interpreter/interpreter.php > $1.~rl

diff_output=$(diff -u -w $1.~ut $1.~rl)
if [ $? -eq 0 ] ; then
  echo "[OK ] $1";
else 
  echo "[ERR] $1";
  echo "${diff_output}"
fi
