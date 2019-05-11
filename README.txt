Library requirements:
	ncurses
	readline
	boost

Installation instructions:
	source env.sh
	#If you have a working pDTS at Boston?:
	#source uhalenv.sh
	#If you have a working pDTS on the np04-srv-* machines:
	#source uhalenv_np04-srv.sh
	# always:
	make


Commands to create a ups product out of this on sbnd-daq33 (based on ProtoDUNE instructions at https://twiki.cern.ch/twiki/bin/view/CENF/WIBArtDAQOp):

cd <base directory of the wibtools repo, i.e., same directory this README.txt is in>
source /grid/fermiapp/products/artdaq/setup
setup gcc v6_4_0
setup boost v1_66_0 -q e15:prof
export QUALIFIER="e15:s64"
export WIBSOFT_VERSION_NO_LEADING_V=<your desired version>  # e.g, "0_00_03"
source env.sh
make clean
make -j8   # 16 cores on sbnd-daq33
cp -rp ups_package/wibtools/v${WIBSOFT_VERSION_NO_LEADING_V}* <existing products directory>/wibtools   

...in this last command, we're assuming there's already a wibtools
product area in <existing products directory>, otherwise copy over the
entire wibtools directory





