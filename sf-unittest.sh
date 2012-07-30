#/bin/sh

if [ $# -ne 1 ] ; then
	echo "Usage: $0 <sf.name>";
	exit;
fi

# Use colordiff if available
diff_bin=`which colordiff`
if [ -z "$diff_bin" ] ; then
  diff_bin="diff";
fi


# Parse output from the source file
sed -n '/\/* Output/,\/*\// { /\* Output/d ; /*\//d ; p } ' $1 > $1.~ut

# Run source file
./src/saffire $1 | php ./interpreter/interpreter.php > $1.~rl

# Fetch output from diff
diff_output=`${diff_bin} -u -w $1.~ut $1.~rl`
if [ $? -eq 0 ] ; then
  echo "[OK ] $1";
  exit 0;
else 
  echo "[ERR] $1";
  echo "${diff_output}"
  exit 1;
fi
